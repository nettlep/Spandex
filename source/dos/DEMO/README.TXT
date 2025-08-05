

The quickest way to get up and running is to:

Make sure the library has been compiled (see ..\source\readme.txt) then:

copy ..\maps\skyenv.tga default.tga
copy skull.usd demo.u
makeit

Run the demo...

------------------------------------------------------------------------------

The self-contained EXE building process (what that make does):

1.  First, it compiled the code into a DEMO.EXP file instead of   
    DEMO.EXE.
2.  After that, it created a DEMO.MAP file by running MAKEMAP on the   
    *.TGA files (DEFAULT.TGA in this case).
3.  Then it compressed the map file onto itself.
4.  Then it compressed the DEMO.U file into DEMO.USD.
5.  Next, it took those two data files (MAP and USD) and stuck them into   
    an STF file (like a WAD file).
6.  And finally, it concatenated the EXP and the STF files into a single   
    EXE file.

You can take this EXE file now and run with it.  You don't need any other   
files to run this other than the EXE itself, since it's all-inclusive.

Note:  If something breaks down, and doesn't work (for example, you   
forgot a mouse when you ran the demo), make sure to look for a LATEX.LOG   
file.  It's actually a good idea to make a batch that runs the demo and   
types the log file if it exists, so you'll always see any errors or   
memory leaks that may not be apparent when you're running the thing.




This engine and the source within is maintained at grafix3d.dyn.ml.org.

- PDN
