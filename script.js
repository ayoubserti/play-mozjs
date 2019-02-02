var dbg = Debugger(g);
dbg.onNewScript = function (s) {
    print('New Script');
};

dbg.onDebuggerStatement = function (frame) {
    print('Found a debugger statment');
    var d = readline();
    print(d);
    try {
        print(frame.url);
    } catch (e) {
        print('Error');
    };
}