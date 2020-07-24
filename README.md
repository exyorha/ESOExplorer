# ESOBrowser

ESOBrowser is a graphical application that allows The Elder Scrolls Online game
client data to be explored: filesystem structure can be inspected, 3D models
and textures may be viewed, text files may be read in plain form. For further
analysis, everything may be exported. ESOBrowser also allows client database
records to be viewed.

Please note that the same caveat regarding database support, as for ESOData,
also applies here: the only supported database format is that of currently
outdated client versions 5.2.11 - 5.2.13, and most of the field names are
missing, and even these that aren't, may be wrong.

# Building

ESOBrowser may be built using normal CMake procedures, and requires Qt 5 and
Google Filament SDKs to be present in the system.

Please note that ESOBrowser uses git submodules, which should be retrieved
before building.

Also note that ESOBrowser uses ESOData for the file access itself, which has
an important caveat regarding a third-party library that needs to be acquired.
See that repository for more details.

# Licensing

ESOBrowser is licensed under the terms of the MIT license (see LICENSE).

Note that ESOBrowser also references MIT-licensed ESOData as a submodule.

