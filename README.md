
IMGUTIL
=======

Imgutil is an utility that can hide several files in a jpeg/png or  automatically downloads a jpeg/png from an url 
and dispatchs the files inside it.
It is very simple, you can add any data you want at the end of a jpeg/png file it will not change the image appearance
or integrity. A basic technique is to rename the extension image file to .rar and add the data inside it then rename 
to .jpeg/.png. It is not very interesting because you simply need to rename the extension to .rar to retrieve the files.
Here the hidden files can only be retrieved by using the associated dispatcher program.


Command Line
------------

dropperCreator <imageFileName> [fileToHide1] [fileToHide2] ... [fileToHideN]

loader <destinationPath> (<imagePath> | <websiteName> <imageUrl>)


SOLUTION
========

dropperCreator
--------------

DropperCreator hides files in the jpeg/png, the project produces a command line program that take the jpeg/png filename
as first parameter and the file to hide names as next one parameters.
The files to hide are simply attached and crypted, then the buffer is append at the end of the jpeg/png file.


loader
------

Loader is the hidden files dispatcher. It can be used to directly dispatch files from an image in the specified directory
but it can also take a server name and an url to download an image and dispatch file inside it.

