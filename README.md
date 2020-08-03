![CodeQL scan](https://github.com/jedisct1/encpipe/workflows/CodeQL%20scan/badge.svg)

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

`-P password_file` can be used to read the password, or an arbitrary
long key (that doesn't have to be text) from a file.

If you don't feel inspired, `-G` prints a random password.

Example - encrypted file transfer:

```sh
nc -l 6666 | encpipe -d -p password
encpipe -e -p password -i /etc/passwd | nc 127.0.0.1 6666
```

Example - compressed, encrypted archives:

```sh
zstd -5 -v -c "$FILE" | encpipe -e -p "$PASSWD" -o "${FILE}.zst.encpipe"
```

# Dependencies

None. It includes [libhydrogen](https://libhydrogen.org) as a
submodule. There is nothing to install.

# Installation

```sh
make
sudo make install
```

# Why

It was faster to write than remember how to use GnuPG and OpenSSL.
