# NISTDownsampler

The NISTDownsampler is an application which will downsample a 1000 PPI fingerprint image to 500 PPI using a Guassian filter followed by decimation, based on the method officially recommended in NIST IR7839 and NIST SP500-289.

For more information, please see [NIST IR7839](http://nvlpubs.nist.gov/nistpubs/ir/2013/NIST.IR.7839.pdf) and [NIST SP500-289](http://nvlpubs.nist.gov/nistpubs/specialpublications/NIST.SP.500-289.pdf).


## Contact
Developer:    John Grantham (john.grantham@nist.gov)  
Project Lead: John Libert (john.libert@nist.gov)


## Visual Studio 2013 Build Instructions

1. Open the Solution File (NISTDownsampler.sln)
2. Check that the build mode is set to "Release"
3. Click "Build Solution" under the "Build" menu option
4. Check the Visual Studio output console for any errors (warnings are OK)
5. If the build is successful, the resulting executable (NISTDownsampler.exe) will be placed in the "bin" sub-directory

NOTE: If you plan to distribute or copy the NISTDownsampler.exe, the .DLL files in the bin directory must be distributed along with the executable (and the target machine may also require the Visual Studio 2013 redistributable package: https://www.microsoft.com/en-us/download/details.aspx?id=40784)


## Disclaimer
See the NIST disclaimer at https://www.nist.gov/disclaimer
