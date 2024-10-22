        Compiling NeoCrypt
        ~~~~~~~~~~~~~~~~~~
    
Requirements :-
---------------

- Microsoft VC 6 or VC.NET (Resuires MFC and hence other compilers
  are not supported)

- STLport 4.5.3 or higher for MSVC 6
  (source available freely at stlport.com)

- cryptlib Security Toolkit (GPL source available freely at
  http://www.cs.auckland.ac.nz/%7Epgut001/cryptlib/)

For MSVC 6 users only:  
You will have to compile the STLport library. Instructions are
provided along with the source distribution which can be downloaded
from http://stlport.com/. After compiler make sure the STLport
include and lib directories are accessible to your compiler.

For everyone:
You will also need to compile the cryptlib library  which can be
easily done with the help of the MVC6 project files provided along
with it.

NeoCrypt assumes that cryptlib source distribution is installed in
"c:\cryptlib". If not then you will need to change this path in
NeoCrypt Project Settings.

If you have any problems/bugs/suggestions, send them to
<arijit1985@yahoo.co.in>.



Arijit De.