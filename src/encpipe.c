#include "encpipe_p.h"

static struct option getopt_long_options[] = {
    { "help", 0, NULL, 'h' },     { "decrypt", 0, NULL, 'd' }, { "encrypt", 0, NULL, 'e' },
    { "in", 1, NULL, 'i' },       { "out", 1, NULL, 'o' },     { "pass", 1, NULL, 'p' },
    { "passfile", 1, NULL, 'P' }, { "passgen", 0, NULL, 'G' }, { NULL, 0, NULL, 0 }
};
static const char *getopt_options = "hdeGi:o:p:P:";

static void
usage(void)
{
    puts(
        "Usage:\n"
        "    encpipe -G\n"
        "    encpipe {-e | -d} {-p <string> | -P <file>} [-i <file>] [-o <file>]\n"
        "\n"
        "Options:\n"
        "    -G, --passgen          generate a random password\n"
        "    -e, --encrypt          encryption mode\n"
        "    -d, --decrypt          decryption mode\n"
        "    -p, --pass <password>  use <password>\n"
        "    -P, --passfile <file>  read password from <file>\n"
        "    -i, --in <file>        read input from <file>\n"
        "    -o, --out <file>       write output to <file>\n"
        "    -h, --help             print this message");
    exit(2);
}

static int
file_open(const char *file, int create)
{
    int fd;

    if (file == NULL || (file[0] == '-' && file[1] == 0)) {
        return create ? STDOUT_FILENO : STDIN_FILENO;
    }
    fd = create ? open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644) : open(file, O_RDONLY);
    if (fd == -1) {
        die(1, "Unable to access [%s]", file);
    }
    return fd;
}

static void
derive_key(Context *ctx, char *password, size_t password_len)
{
    static uint8_t master_key[hydro_pwhash_MASTERKEYBYTES] = { 0 };

    if (ctx->has_key) {
        die(0, "A single key is enough");
    }
    if (hydro_pwhash_deterministic(ctx->key, sizeof ctx->key, password, password_len, HYDRO_CONTEXT,
                                   master_key, PWHASH_OPSLIMIT, PWHASH_MEMLIMIT,
                                   PWHASH_THREADS) != 0) {
        die(0, "Password hashing failed");
    }
    hydro_memzero(password, password_len);
    ctx->has_key = 1;
}

static int
stream_encrypt(Context *ctx)
{
    unsigned char *const chunk_size_p = ctx->buf;
    unsigned char *const chunk        = chunk_size_p + 4;
    uint64_t             chunk_id;
    ssize_t              max_chunk_size;
    ssize_t              chunk_size;

    assert(ctx->sizeof_buf >= 4 + hydro_secretbox_HEADERBYTES);
    max_chunk_size = ctx->sizeof_buf - 4 - hydro_secretbox_HEADERBYTES;
    assert(max_chunk_size <= 0x7fffffff);
    chunk_id = 0;
    while ((chunk_size = safe_read_partial(ctx->fd_in, chunk, max_chunk_size)) >= 0) {
        STORE32_LE(chunk_size_p, (uint32_t) chunk_size);
        if (hydro_secretbox_encrypt(chunk, chunk, chunk_size, chunk_id, HYDRO_CONTEXT, ctx->key) !=
            0) {
            die(0, "Encryption error");
        }
        if (safe_write(ctx->fd_out, chunk_size_p, 4 + hydro_secretbox_HEADERBYTES + chunk_size,
                       -1) < 0) {
            die(1, "write()");
        }
        if (chunk_size == 0) {
            break;
        }
        chunk_id++;
    }
    if (chunk_size < 0) {
        die(1, "read()");
    }
    return 0;
}

static int
stream_decrypt(Context *ctx)
{
    unsigned char *const chunk_size_p = ctx->buf;
    unsigned char *const chunk        = chunk_size_p + 4;
    uint64_t             chunk_id;
    ssize_t              readnb;
    ssize_t              max_chunk_size;
    ssize_t              chunk_size;

    assert(ctx->sizeof_buf >= 4 + hydro_secretbox_HEADERBYTES);
    max_chunk_size = ctx->sizeof_buf - 4 - hydro_secretbox_HEADERBYTES;
    assert(max_chunk_size <= 0x7fffffff);
    chunk_id = 0;
    while ((readnb = safe_read(ctx->fd_in, chunk_size_p, 4)) == 4) {
        chunk_size = LOAD32_LE(chunk_size_p);
        if (chunk_size > max_chunk_size) {
            die(0, "Chunk size too large ([%zd] > [%zd])", chunk_size, max_chunk_size);
        }
        if (safe_read(ctx->fd_in, chunk, chunk_size + hydro_secretbox_HEADERBYTES) !=
            chunk_size + hydro_secretbox_HEADERBYTES) {
            die(0, "Chunk too short ([%zd] bytes expected)", chunk_size);
        }
        if (hydro_secretbox_decrypt(chunk, chunk, chunk_size + hydro_secretbox_HEADERBYTES,
                                    chunk_id, HYDRO_CONTEXT, ctx->key) != 0) {
            fprintf(stderr, "Unable to decrypt chunk #%" PRIu64 " - ", chunk_id);
            if (chunk_id == 0) {
                die(0, "Wrong password or key?");
            } else {
                die(0, "Corrupted or incomplete file?");
            }
        }
        if (chunk_size == 0) {
            break;
        }
        if (safe_write(ctx->fd_out, chunk, chunk_size, -1) < 0) {
            die(1, "write()");
        }
        chunk_id++;
    }
    if (readnb < 0) {
        die(1, "read()");
    }
    if (chunk_size != 0) {
        die(0, "Premature end of file");
    }
    return 0;
}

static int
read_password_file(Context *ctx, const char *file)
{
    char    password_[512], *password = password_;
    ssize_t password_len;
    int     fd;

    fd = file_open(file, 0);
    if ((password_len = safe_read(fd, password, sizeof password_)) < 0) {
        die(1, "Unable to read the password");
    }
    while (password_len > 0 &&
           (password[password_len - 1] == ' ' || password[password_len - 1] == '\r' ||
            password[password_len - 1] == '\n')) {
        password_len--;
    }
    while (password_len > 0 && (*password == ' ' || *password == '\r' || *password == '\n')) {
        password++;
        password_len--;
    }
    if (password_len <= 0) {
        die(0, "Empty password");
    }
    close(fd);
    derive_key(ctx, password, password_len);

    return 0;
}

static void
passgen(void)
{
    unsigned char password[32];
    char          hex[32 * 2 + 1];

    hydro_random_buf(password, sizeof password);
    hydro_bin2hex(hex, sizeof hex, password, sizeof password);
    puts(hex);
    hydro_memzero(password, sizeof password);
    hydro_memzero(hex, sizeof hex);
    exit(0);
}

static void
options_parse(Context *ctx, int argc, char *argv[])
{
    int opt_flag;
    int option_index = 0;

    ctx->encrypt = -1;
    ctx->in      = NULL;
    ctx->out     = NULL;
    optind       = 0;
#ifdef _OPTRESET
    optreset = 1;
#endif
    while ((opt_flag = getopt_long(argc, argv, getopt_options, getopt_long_options,
                                   &option_index)) != -1) {
        switch (opt_flag) {
        case 'd':
            ctx->encrypt = 0;
            break;
        case 'e':
            ctx->encrypt = 1;
            break;
        case 'G':
            passgen();
            break;
        case 'i':
            ctx->in = optarg;
            break;
        case 'o':
            ctx->out = optarg;
            break;
        case 'p':
            derive_key(ctx, optarg, strlen(optarg));
            break;
        case 'P':
            read_password_file(ctx, optarg);
            break;
        default:
            usage();
        }
    }
    if (ctx->has_key == 0 || ctx->encrypt == -1) {
        usage();
    }
}

int
main(int argc, char *argv[])
{
    Context ctx;

    if (hydro_init() < 0) {
        die(1, "Unable to initialize the crypto library");
    }
    memset(&ctx, 0, sizeof ctx);
    options_parse(&ctx, argc, argv);
    ctx.sizeof_buf = DEFAULT_BUFFER_SIZE;
    if (ctx.sizeof_buf < MIN_BUFFER_SIZE) {
        ctx.sizeof_buf = MIN_BUFFER_SIZE;
    } else if (ctx.sizeof_buf > MAX_BUFFER_SIZE) {
        ctx.sizeof_buf = MAX_BUFFER_SIZE;
    }
    if ((ctx.buf = (unsigned char *) malloc(ctx.sizeof_buf)) == NULL) {
        die(1, "malloc()");
    }
    assert(sizeof HYDRO_CONTEXT == hydro_secretbox_CONTEXTBYTES);

    ctx.fd_in  = file_open(ctx.in, 0);
    ctx.fd_out = file_open(ctx.out, 1);
    if (ctx.encrypt) {
        stream_encrypt(&ctx);
    } else {
        stream_decrypt(&ctx);
    }
    free(ctx.buf);
    close(ctx.fd_out);
    close(ctx.fd_in);
    hydro_memzero(&ctx, sizeof ctx);

    return 0;
}
