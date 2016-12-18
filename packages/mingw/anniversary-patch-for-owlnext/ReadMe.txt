Anniversary Patch for OWLNext

After the Anniversary Update for Windows 10 (i.e. version 1607), applications
linked dynamically with OWLNext in Unicode mode would fail with a System Error
saying "The program can't start because ext-ms-win-gdi-desktop-l1-1-0.DLL is
missing from your computer". The same error would appear for applications that
use static linking in Unicode mode, if TPrintDC::DeviceCapabilities is used.

This project creates "ext-ms-win-gdi-desktop-l1-1-0.DLL" in which the missing
function is defined as a stub. Deploy the DLL with the OWLNext application to
suppress the system error message.

Note that the missing function is just a stub that returns -1 and sets the 
system error state to ERROR_CALL_NOT_IMPLEMENTED. If the application actually
uses TPrintDC::DeviceCapabilities, it may no longer work.

For more information, see: http://sourceforge.net/p/owlnext/bugs/342
