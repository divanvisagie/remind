# remind

A small Unix-style reminder manager, inspired by BSD’s [`mail(1)`](https://man.openbsd.org/mail.1).

Reminders are stored as plain text, one per line, in:

`$HOME/.local/state/remind/reminders`

This makes it easy to read, edit, back up, or even sync across systems.

---

## Features

- **Check** reminders with `remind -c`
- **Add** reminders with `remind -a "Buy milk"`
- **Delete** reminders with `remind -d 2`
- **Edit** reminders directly in `nvim` (just run `remind` with no flags)
- **Plain text storage** — nothing hidden, no databases, no lock-in
- **Inspired by BSD `mail(1)`** — simple commands, simple storage

---

## Installation

Clone the repo and install:

```sh
git clone https://github.com/divanvisagie/remind.git
cd remind
make
sudo make install
````

By default, this installs the binary into `/usr/local/bin` and the man page into `/usr/local/share/man/man1`.

---

## Usage

Add a new reminder:

```sh
remind -a "Buy milk"
```

Check your reminders:

```sh
remind -c
```

Delete the second reminder:

```sh
remind -d 2
```

Edit reminders manually in `nvim`:

```sh
remind
```

---

## Pro tip

Add this to your shell config (e.g. `.zshrc`) so you see reminders every time you start a shell:

```sh
remind -c
```

---

## Man page

Once installed, you can read the full documentation with:

```sh
man remind
```

---

## Author

Divan Visagie, 2025.
Inspired by BSD’s `mail(1)`.

