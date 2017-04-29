import subprocess

class Part:
    def __init__( self ):
        self.file = ""
        self.name = "default"
        self.delta = 0.0
        self.beta = 0.0

class Mesher:
    def __init__( self ):
        self.parts = []
        self.meshname = ""

    def mesh( self, outfile, clscale=1.0, show=False ):
        # Prepare GMSH file
        out = open( outfile, 'w' )
        for part in self.parts:
            out.write("Merge ""%s"";\n"%(part.file))

        counter = 0
        for part in self.parts:
            out.write("Physical Volume(""%s"")={%d};\n"%(part.name,counter))
            counter += 1
        out.close()

        self.meshname = outfname.split(".")[0]
        self.meshname += ".msh"
        subprocess.call(["gmsh","-i", outfile,"-clscale","%.3f"%(clscale),"-3","-o",self.meshname])

        if ( show ):
            subprocess.call(["gmsh", self.meshname])

    def getMaterialArray( self ):
        deltaArray = []
        betaArray = []
        for part in self.parts:
            deltaArray.append( part.delta )
            betaArray.append( part.beta )
        return deltaArray, betaArray
