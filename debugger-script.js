var dbg = Debugger(__globalObjectProxy);
WebSocket.run();
dbg.onNewScript = function (s) {
    var frame = dbg.getNewestFrame()
    print(frame.type);
    print('New Script');
};
dbg.onDebuggerStatement = function (frame) {
    //if(frame.onStep) return;
    print('Found a debugger statment');
    var d = readline();
    while (d != 'c') {
        try {
            print(d);
            frame.eval(d)
            var msg = WebSocket.pop_event()
            if ( msg ){
                print(msg)
            }
            WebSocket.send_event("help");
            d = readline();
        }
        catch (e) {
            print("error")
        }



    }

}

dbg.onEnterFrame = function(frame){
    print("frame from " + frame.type)
}