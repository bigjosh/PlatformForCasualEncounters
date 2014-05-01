// This will convert a linux shell script into a C snippet that you can
// paste into an Arduino sketch and download to a Yun.

// Note: Don't use sheel variables becuase the Bridge will run each command in a new envronment

def name =  /D:\Users\josh\Documents\GitHub\PlatformForCasualEncounters\BlinkMe\misc\BlinkMeSetup.sh/


def sourceShell = new File(name )  

def EOMMode = false


println( "// What follows is code automatically generated form the shell script "+ sourceShell.getName() )
println( "// ==========================================================================================================" )

println()
println("#define NL \"\\n\"")
println()

sourceShell.eachLine {

    def stripped = it.trim(); 
    
    if (stripped.length() == 0 ) {                // Blank lines stay blank lines
    
        println("")
    
    } else if (stripped.startsWith('#') && !stripped.startsWith('#!') ) {            // Check if comment and not a Shebang, turn into a C comment
    
        println( "//" +it.replace('#', "" ) )
        
        
    } else {            // Ok, it is a line of script...
    
    
         if (stripped.endsWith("<<EOM") || stripped.endsWith("<<'EOM'") ) {   // Here document must be a single commmand to the bridge

            // We are starting a here doc
                     
            println("   run( F(\""+it+"\" NL")
            println("   //-----")            
            
            EOMMode = true
            
         } else if ( EOMMode && stripped.startsWith("EOM") ) { 
         
            // We are ending a here doc
            
            println("   //-----")            
            println("    \""+it+"\" NL));")
                
            EOMMode = false            
            
         } else {         
         
             if (EOMMode) {
             
                println("   \""+it+"\" NL")
             
             
             } else {
             
                println("   run( F(\""+it+"\"));" )
                
             }
         }
    }        
}

println()
println( "// ==========================================================================================================" )

println( "// End of code automatically generated from the shell script "+ sourceShell.getName() )
