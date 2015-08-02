# Spreadsheet Cleaner

The purpose of this script is to delete duplicate rows from a spreadsheet file. It should be able to deal with CSV and Excel files.

## Installation
You will need a working [IPython Notebook](http://ipython.org/notebook.html) installation.

### OSX
* python should already be installed
* install easy_install/setuptools by running ```curl https://bootstrap.pypa.io/ez_setup.py -o - | sudo python``` (https://pypi.python.org/pypi/setuptools) in a terminal
* install pip via ```sudo easy_install pip```
* get ipython via ```sudo pip install "ipython[all]"``` (https://ipython.org/ipython-doc/dev/install/install.html)

You should now be able to start IPython Notebook via ```ipython notebook```

For excel support you additionally need [openpyx](http://openpyxl.readthedocs.org/en/latest/index.html), which you can install via ```sudo pip install openpyxl```
