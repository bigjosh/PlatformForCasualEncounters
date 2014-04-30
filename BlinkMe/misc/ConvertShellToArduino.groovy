// This will convert a linux shell script into a C snippet that you can
// paste into an Arduino sketch and download to a Yun.

def name =  /D:\Users\josh\Documents\GitHub\PlatformForCasualEncounters\BlinkMe\misc\BlinkMeSetup.sh/


def sourceShell = new File(name )  



sourceShell.eachLine {

    def stripped = it.stripIndent(); 
    
    if (stripped.trim().length() == 0 ) {
    
        println("")
    
    } else if (stripped.startsWith('#') && !stripped.startsWith('#!') ) {
    
        println( "//" +it.replace('#', "" ) )
        
    }
    
    else {
        println("   run( F(\""+it+"\"));" )
    }        
}

