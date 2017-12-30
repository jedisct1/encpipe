Encpipe
=======

The dum^H^H^Hsimplest encryption tool in the world.

# Usage

Encrypt a file using a password:

```sh
encpipe -e -p password -i inputfile -o outputfile
```

Decrypt a file using a password:

```sh
encpipe -d -p password -i inputfile -o outputfile
```

`-i` and `-o` can be set to `-` or omitted to read/write from the
standard input/output.

Example - encrypted file transfer:

```sh
nc -l 6666 | encpipe -d -p password
encpipe -e -p password -i /etc/passwd | nc 127.0.0.1 6666
```

Example - compressed, encrypted archives:

```sh
zstd -5 -v -c "$FILE" | encpipe -e -p "$PASSWD" -o "${FILE}.zstd.encpipe"
```

# Dependencies

None. It includes [libhydrogen](https://github.com/jedisct1/libhydrogen) as
a submodule. There is nothing to install.

# Installation

```sh
make
sudo make install
```

[Precompiled, static binaries for Linux](https://github.com/jedisct1/encpipe/releases/latest) are also available.

Use `gmake` instead of `make` on BSD systems.

# Why

It was faster to write than remember how to use GnuPG and OpenSSL.

# Yes

It's using passwords. Provided on the command-line. Yes, it actually
does that. I needed that. Right now. Other options will be
implemented. Next year.
