# Like Brainfuck, but with directional control!
Ever thought Brainfuck was too easy to read, or understand? Directional Brainfuck solves those issues. Instead of reading left-to-right, one-directionally, dbrainfuck bravely broke out into the 2nd dimension.
The program runs exactly how it looks: it can read up, or down, or left, or right, depending on what direction you want it to run.

### Here are the valid commands in dbrainfuck:
* `+` - Same as brainfuck
* `-` - Same as brainfuck
* `.` - Same as brainfuck
* `,` - Same as brainfuck
* `{` - Equivalent to brainfuck's `<`
* `}` - Equivalent to brainfuck's `>`
* `>` - Changes direction to left-to-right
* `<` - Changes direction to right-to-left
* `^` - Changes direction to up
* `v` - Changes direction to down
* `?` - Conditional. If the current byte is `0`, changes direction to `^`. Otherwise, changes direction to `v`.
* `d` - Toggle debug mode in-program
* `e` - Exit program

### Loops? Conditionals?
Because `[` and `]` have been removed, you have to get creative with loops. You would write `[-]` as:
```
> v
>-?>
^ <
```
Likewise, a simple `cat` implementation in brainfuck would look like `,+[-.,+]`. In directional brainfuck, it would be:
```
v       <
> +>-,.+?
   ^    <
```

A lot of examples can be found in the `examples/` directory.

## Building & Running
Run `make` to build.

To run, execute `./dbf [-d] FILE`. The -d argument enables debug mode by default.
