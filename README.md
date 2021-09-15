# mopatop
Model Of Agents and Their Occupied Places.
See html/index.html for documentation - preview this with
https://htmlpreview.github.io/?https://github.com/mikbitz/mopatop/blob/main/html/index.html

and for tests

https://htmlpreview.github.io/?https://github.com/mikbitz/mopatop/blob/main/tests/html/index.html

although note that not all links in the above page menus work directly with this method.

The model has been tested and runs on opensuse linux with g++ version > 9 

and on macOS 11.5.2 BigSur using g++-11 installed with homebrew as follows:-

/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

brew reinstall gcc

Note that this requires that you explicitly use g++-11 to compile, as the default g++ seems not to support openmp.
