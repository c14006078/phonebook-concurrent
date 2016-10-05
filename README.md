# Summary

* Learn POSIX Thread Programming
* Realize the software optimizations
* Code refactoring

# Indenting Code
You can use AStyle ([Artistic Style](http://astyle.sourceforge.net/)) source code indenter to
help you auto format your source code. It will for sure not correct all your coding styles but
for sure will eliminate most of them. You can download AStyle from [this location](http://astyle.sourceforge.net/)
or install via `apt-get`:
```sh
sudo apt-get install astyle
```

To format your file you can execute below command:
```sh
astyle --style=kr --indent=spaces=4 --indent-switches --suffix=none *.[ch]
```

Install Git pre-commit hook to check C/C++ source file format
```sh
scripts/install-git-hooks
```
#Makefile flag

Debug version: enable the dprintf()
```sh
make DEBUG=1
```
Timing version: timing each of threads
```sh
make TIMING=1
```

# Licensing
`phonebook` is freely redistributable under the two-clause BSD License.
Use of this source code is governed by a BSD-style license that can be found
in the `LICENSE` file.
