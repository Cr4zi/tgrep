# tgrep
A tiny POC implementation of grep.

## Syntax
Base expressions are any chars in the ASCII table (there are some escaped characters). \
To show the syntax we'll denote one expression as E, and second expression as F.

```
EF - concatenating E and F.
E + F - E or F.

*E - one char then a word from E.
E* - a word from E then one char.

E^i - concatenating E i times. (i has to be integer)

For example you could do: *^i
```

### Order of operations
For the sake of simplicity there is an order of operations. The order is:
1. ()
2. ^
3. \*
4. \+

### Escaped characters
Those are special characters that in order to write them inside an expression without using their functionality above you have to escape them: \
- \\*
- \\^
- \(
- \)


## Build from source:
``` sh
$ git clone https://github.com/Cr4zi/tgrep.git
$ cd tgrep
$ make
```

## Why?
Just wanted to implement something I saw recently at a lecture that I thought was really interesting.

## Should you use it?
Absolutely not. This is just POC and not something that should be used.
