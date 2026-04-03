#!/usr/bin/python3

from sys import argv, exit

"""
Automatically translate brainfuck into directional brainfuck
Usage: ./bf2dbf.py [file]
If no file is given, read from stdin.
Directional brainfuck will be printed to stdout.
"""

def translate_bf(bf):
    bf_chars=",.+-<>[]"

    tmp_bf = "".join([char for char in bf if char in bf_chars])
    tmp_bf = tmp_bf.replace("[", "[ ").replace("]", "] ").replace(">", "}").replace("<", "{")
    fixed_bf = ">" + tmp_bf

    istart = []  # stack of indices of opening brackets
    loops = {}

    # Thank you, stack exchange!
    for i, c in enumerate(fixed_bf):
        if c == '[':
            istart.append(i)
        if c == ']':
            try:
                loops[istart.pop()] = i
            except IndexError:
                print('Too many closing brackets')
                exit(1)

    if istart:  # check if stack is empty afterwards
        print('Too many opening brackets')
        exit(1)

    fixed_bf=fixed_bf.replace("[ ", "?>").replace("] ", "?>")
    top_lines = []
    bottom_lines = []
    result = ""

    # Reversed here so that deeply nested loops have the smallest travel path
    for loop in reversed(loops):
        l_start=loop
        l_end=loops[loop]
        l_len = l_end - l_start - 1
        top_lines.append(" " * l_start + ">" + " " * l_len + ">v");

    # This doesn't need to be reversed
    for loop in loops:
        l_start=loop
        l_end=loops[loop]
        l_len = (l_end - l_start) - 2
        bottom_lines.append(" " * l_start + ">^" + " " * l_len + "<")

    # If anything was actually added, we need to prepend a "v" so directional
    # brainfuck knows to travel down first
    if len(top_lines) > 0 and len(top_lines[0]) > 0:
        top_lines[0] = "v" + top_lines[0][1:]

    result = "\n".join(top_lines)
    result += "\n" + fixed_bf + "\n"
    result += "\n".join(bottom_lines)

    return result

def read_from_stdin():
    bf = ""
    for line in stdin:
        bf += line

    return bf

def read_from_file():
    try:
        with open(sys.argv[1], 'r') as file:
            bf = file.read()
    except Exception as e:
        print(e)
        exit(1)
        
    return bf
    
def main():
    if len(sys.argv) < 2:
        bf = read_from_stdin()
    else:
        bf = read_from_file(argv[1]);
        if bf == -1:
            return 1

    translated = translate_bf(bf)

    print(translated)
    
if __name__ == '__main__':
    main()
