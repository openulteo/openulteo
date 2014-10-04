==============
FreeRDP Mobile
==============

Get the code
------------

Clone the main code : git clone git@devel.ulteo.com:ovd-ipad
Init the submodule : git submodule init
Fetch the submodule content : git submodule update

Generate the i18n
-----------------

git svn clone https://svn.ulteo.com/ovd/trunk/i18n ovd

easy_install pip
pip install argparse
pip install translate-toolkit

cd i18n/ovd
./svn2xcode.py -s ovd/uovdclient -o ../FreeRDPMobile/FreeRDPMobile/SupportingFiles/i18n


Build the FreeRDP static library
--------------------------------

cd FreeRDP
./autogen.sh
./buildlibios.py -a
