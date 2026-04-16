let lio = (() => {
    let tabData = void 0,
        mapData = void 0,
        jazdaData = void 0,
        isInfoActive = !1,
        errorCode = 0,
        isStopped = !1,
        isCackError = !1;

    let onTabs, onVInfo, onMap, onStateChange, onJazda, onInfoText;

    // Initialize socket connection
    let socket = (
        onTabs = onVInfo = onMap = onStateChange = onJazda = onInfoText = () => {},
        io.connect("https://imhd.sk/", {
            path: "/rt/sio2"
        })
        .on("tabs", (data) => {
            try { onTabs(data) } catch (e) {}
        })
        .on("vInfo", (data) => {
            try { onVInfo(data) } catch (e) {}
        })
        .on("map", (data) => {
            try { onMap(data) } catch (e) {}
        })
        .on("jazda", (data) => {
            try { onJazda(data) } catch (e) {}
        })
        .on("iText", (data) => {
            try { onInfoText(data) } catch (e) {}
        })
        .on("connect", () => {
            if (isStopped || isCackError) {
                forceDisconnect();
            } else {
                errorCode = 0;
                onStateChange(!0);
                requestData();
            }
        })
        .on("disconnect", () => {
            errorCode = -101;
            onStateChange(!1);
        })
        .on("error", (err) => {
            errorCode = -102;
            console.error("SIO error: " + err);
        })
        .on("cack", (data) => {
            if (data !== !0) {
                isCackError = data;
                forceDisconnect();
            }
        })
    );

    function forceDisconnect() {
        socket.disconnect();
        socket.io.reconnection(!1);
    }

    function requestData() {
        if (socket && socket.connected) {
            tabData && socket.emit("tabStart", tabData);
            mapData && socket.emit("mapStart", mapData);
            jazdaData && socket.emit("jazdaStart", jazdaData);
            isInfoActive && socket.emit("infoStart");

            if (!tabData && !mapData && !jazdaData && !isInfoActive) {
                socket.emit("dataStop");
            }
        }
    }

    let controller = {
        start: function() {
            isStopped = !1;
            if (!isCackError && socket) {
                socket.io.reconnection(!0);
                if (!socket.connected) socket.connect();
            }
        },
        stop: function() {
            isStopped = !0;
            socket && forceDisconnect();
        },
        conState: function() {
            if (!socket) return [0, -100];
            if (socket.connected) return [1, 0];
            if (isStopped) return [-1, -103];
            if (isCackError) return [-2, isCackError[0]];
            return [0, errorCode];
        },
        on: function(event, callback) {
            switch (event) {
                case "stateChange": onStateChange = callback; break;
                case "tabs":      onTabs = callback;        break;
                case "vInfo":     onVInfo = callback;       break;
                case "map":       onMap = callback;         break;
                case "jazda":     onJazda = callback;       break;
                case "infoText":  onInfoText = callback;    break;
            }
            return controller;
        },
        vInfo: function(val) {
            socket && socket.emit("vInfo", val);
        },
        tabStart: function(a, b, c) {
            tabData = void 0;
            if (a && b) {
                tabData = [a, b, c];
                requestData();
            }
        },
        mapStart: function(val) {
            mapData = void 0;
            if (val) {
                mapData = val;
                requestData();
            }
        },
        jazdaStart: function(a, b) {
            jazdaData = void 0;
            if (socket) {
                jazdaData = [a, b];
                requestData();
            }
        },
        infoStart: function() {
            isInfoActive = !0;
            requestData();
        },
        refresh: function() {
            requestData();
        },
        dataStop: function() {
            tabData = mapData = jazdaData = void 0;
            isInfoActive = !1;
            requestData();
        }
    };

    return controller;
})();