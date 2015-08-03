# Spreadsheet Cleaner

The purpose of this script is to delete duplicate rows from a spreadsheet file. It should be able to deal with CSV and Excel files. Note that ipython notebooks are **not** python scripts (see *Usage* below) even though the code parts are mostly pure python.

## Installation
You will need a working [IPython Notebook](http://ipython.org/notebook.html) installation.

### OSX
* python should already be installed
* install easy_install/setuptools by running
```
$ curl https://bootstrap.pypa.io/ez_setup.py -o - | sudo python
```
(https://pypi.python.org/pypi/setuptools) in a terminal
* install pip via
```
$ sudo easy_install pip
```
* get ipython via
```
$ sudo pip install "ipython[all]"
```
(https://ipython.org/ipython-doc/dev/install/install.html)

You should now be able to start IPython Notebook via
```
$ ipython notebook
```

For excel support you additionally need [openpyxl](http://openpyxl.readthedocs.org/en/latest/index.html), which you can install via
```
$ sudo pip install openpyxl
```

### Linux
You can most likely install the relevant packages using you package manager. For [Arch Linux](https://www.archlinux.org/) this is
```
$ sudo pacman -S ipython-notebook python-openpyxl
```

## Usage
You will first have to start the ipython notebook server
```
$ ipython notebook
```
This opens a page in you browser where you can navigate to [the notebook](./Spreadsheet Cleaner.ipynb) and open it. You find more instructions within the notebook, which will hopefully enable people with very basic programming knowledge to adapt the script to their needs.
