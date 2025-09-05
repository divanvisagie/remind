# NAME

remind - simple reminder manager

# SYNOPSIS

**remind** \[-c\] \[-a TEXT\] \[-d N\]

# DESCRIPTION

**remind** is a small program to manage personal reminders stored in
*$HOME/.local/state/remind/reminders*.

Reminders are stored as plain text, one per line. The design is inspired
by the classic **mail**(1) program in BSD Unix: simple commands, plain
text storage, and direct editing.

# OPTIONS

**-c**

:   Check reminders. Prints the current list of reminders to stdout.


**-a *TEXT***

:   Add a new reminder line containing *TEXT*. If *TEXT* contains
    spaces, quote it.


**-d *N***

:   Delete reminder at line number *N* (1-based).


**(no options)**

:   Open the reminders file in *$EDITOR* for manual editing. If no
    $EDITOR is set, use *vi*.

# FILES

*$HOME/.local/state/remind/reminders*

:   Storage location of reminders.

# EXAMPLES

Add a reminder:

:   remind -a "Buy milk"


List all reminders:

:   remind -c


Delete the second reminder:

:   remind -d 2


Edit reminders manually in *nvim*:

:   remind


Automatically check reminders every time you start a shell:

:   echo 'remind -c' >> ~/.zshrc

# SEE ALSO

**mail**(1), **nvim**(1), **man**(1)

# HISTORY

The **remind** utility first appeared in 2025. It was inspired by
**mail**(1) in BSD Unix.

# AUTHOR

Divan Visagie, 2025.
