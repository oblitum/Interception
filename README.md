Interception
============

[Download the latest release][latest-release]

Building
--------

Source code is built upon [Windows Driver Kit Version 7.1.0][wdk].

Simple build scripts (`buildit.cmd`) are provided to build using specific build
environments of the WDK, they require the environment variable `%WDK%` to be
previously set to the WDK installation directory.

- Tested from Windows XP to Windows 10.

Driver installation
-------------------

Drivers can be installed through the command line installer, but driver
installation requires execution inside a prompt with administrative rights.

Run `install-interception` without any arguments inside an console executed as
administrator and it will give instructions for installation.

License
-------
Interception is dual-licensed.

For non-commercial purposes it adopts LGPL for the library and its source code,
with rights of distribution of the related binary assets (drivers and installers
) once communication with drivers happen solely by use of the library and its
API.

For commercial purposes it adopts two other licenses which can be checked in the
[licenses directory][licenses]:

 - [Interception API License][interception-api-license]:  
 It's similar to the non-commercial license, solely removing restrictions for
 commercial usage.  
 It also includes an additional asset which is an installer
 library, so that driver installation can be embedded silently in your own
 installer.

 - [Interception License][interception-license]:  
 Provides access to all source code, including drivers and installers.

Please contact me at <francisco@oblita.com> for acquiring a commercial license.

Use cases
---------

Interception has been used around the world in cases I couldn't imagine when I
first created it:

 - Helping people with accessibility limitations, tailoring systems according
 to their limitations.
 - By companies in aviation training, to connect many devices at once and
 customizing each one.
 - By companies providing SCADA (supervisory control and data acquisition)
 solutions.
 - In game applications like BOTs and control customization.
 - To construct an emacs mode of the system.
 - To customize supermarket cashier's systems.
 - In doctoral thesis about typing pattern recognition for security applications.
 - Home theater automation.
 - ...

For fun:

![lissajous][lissajous]
![rose][rose]
![butterfly][butterfly]

© 2008 Francisco Lopes

[latest-release]: https://github.com/oblitum/interception/releases/latest
[wdk]: http://www.microsoft.com/en-us/download/details.aspx?id=11800
[lissajous]: https://sites.google.com/a/oblita.com/yorick/_/rsrc/1237686557679/hooking-part3/lissajous.png
[rose]: https://sites.google.com/a/oblita.com/yorick/_/rsrc/1237686585300/hooking-part3/rose.png
[butterfly]: https://sites.google.com/a/oblita.com/yorick/_/rsrc/1237686591599/hooking-part3/butterfly.png
[licenses]: https://github.com/oblitum/Interception/tree/master/licenses
[interception-license]: https://github.com/oblitum/Interception/raw/master/licenses/commercial-usage/Interception.pdf
[interception-api-license]: https://github.com/oblitum/Interception/raw/master/licenses/commercial-usage/Interception%20API.pdf
