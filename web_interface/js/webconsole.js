Output=getE("Output")
Command=getE("Command")
CLIHistory=[]
HistoryPointer=0
Command.addEventListener('keyup',(ev)=>{
    if(ev.keyCode==13){
        //Enter is pressed
        cmd=Command.value
        if (cmd.length>0) {
            if(cmd.trim()=="clear"){
                Output.value=""
                Command.value=""
            }else{
                getFile(`/run?cmd=${cmd}`,()=>{
                    CLIHistory.push(cmd)
                    Command.value=""
                    HistoryPointer=0
                    getFile("/console",(responseText)=>{
                        Output.value=responseText+Output.value
                    })
                })
            }
        }
    }

    else if(ev.keyCode==38){
        //Up arrow key presssed
        if(CLIHistory.length>0){
            len=CLIHistory.length
            if(CLIHistory[len-HistoryPointer-1]){
                Command.value=CLIHistory[len-HistoryPointer-1]
                HistoryPointer++
            }
        }
    }
    else if(ev.keyCode==40){
        //Down arrow key pressed
        if(CLIHistory.length>0){
            len=CLIHistory.length
            if(CLIHistory[len-HistoryPointer+1]){
                Command.value=CLIHistory[len-HistoryPointer+1]
                HistoryPointer--
            }
        }
    }
    else{
        HistoryPointer=0
    }
})

setInterval(()=>{
    getFile("/console",(responseText)=>{
        Output.value=responseText+Output.value
    })
},5000)