/**
 * Dummy Socket.IO v4 server mimicking imhd.sk real-time transit data server.
 *
 * Observed traffic from imhd.sk:
 *   - Path:    /rt/sio2
 *   - Events:  tabs, tabStart, vInfo
 *
 * Run:  node server.js
 * Port: 3000 (override with PORT env var)
 */

const { createServer } = require("http");
const { Server } = require("socket.io");

const PORT = process.env.PORT || 3000;

// ---------------------------------------------------------------------------
// HTTP server + Socket.IO setup
// ---------------------------------------------------------------------------
const httpServer = createServer();

const io = new Server(httpServer, {
  // Match the path used by imhd.sk
  path: "/rt/sio2",

  // Allow any origin (CORS open for local dev)
  cors: {
    origin: "*",
    methods: ["GET", "POST"],
  },

  // Engine.IO v4 is the default for Socket.IO v4 — no extra config needed
});

// TODO replace by binary-response-example.txt
const RANDOM_TABS = require("../tabs-message-example.json")[1];

// ---------------------------------------------------------------------------
// Connection handler
// ---------------------------------------------------------------------------
io.on("connection", (socket) => {
  console.log(`[connect]  socket=${socket.id}`);

  // Per-socket map of active vInfo intervals keyed by "<zastavka>:<nastupiste>"
  const vInfoIntervals = new Map();

  // -------------------------------------------------------------------------
  // Handshake / welcome
  // Emit a lightweight acknowledgement so the client knows the server is ready.
  // -------------------------------------------------------------------------
  socket.emit("welcome", {
    server: "imhd-dummy",
    version: "1.0.0",
    timestamp: Date.now(),
  });

  // -------------------------------------------------------------------------
  // `tabStart` event
  //
  // Client sends:
  //   [zastavka, [nastupiste, ...], region]
  //   e.g. [355, [864], "ba"]  or  [355, ["*"], "ba"]
  //
  // Server starts emitting periodic `tabs` events 
  // -------------------------------------------------------------------------
  socket.on("tabStart", (zastavka, nastupiste, region) => {
    console.log(
      `[tabStart] socket=${socket.id}`,
      JSON.stringify({ zastavka, nastupiste, region })
    );

    // const platforms = Array.isArray(nastupiste) ? nastupiste : [nastupiste];

    // platforms.forEach((platform) => {
      const key = `${zastavka}`;

      // Avoid duplicate intervals for the same subscription
      if (vInfoIntervals.has(key)) {
        console.log(`  -> already subscribed to ${key}, skipping`);
        return;
      }

      console.log(`  -> starting tabs stream for key=${key} ooo`);

      // Emit an immediate first update, then continue every 30 s
      socket.emit("tabs", RANDOM_TABS);

      const interval = setInterval(() => {
        if (!socket.connected) {
          clearInterval(interval);
          vInfoIntervals.delete(key);
          return;
        }
        // const vehicle = randomVehicle();
        // socket.emit("vInfo", vehicle);
        const tabs = RANDOM_TABS;
        socket.emit("tabs", tabs);
        console.log(
          `[tabs]    socket=${socket.id} key=${key}}`
        );
      }, 30000);

      vInfoIntervals.set(key, interval);
    // });
  });

  // -------------------------------------------------------------------------
  // `tabStop` event  (graceful unsubscribe — not observed but good practice)
  //
  // Client sends: [zastavka, [nastupiste, ...]]
  // Server stops the corresponding vInfo stream.
  // -------------------------------------------------------------------------
  socket.on("tabStop", (zastavka, nastupiste) => {
    console.log(
      `[tabStop]  socket=${socket.id}`,
      JSON.stringify({ zastavka, nastupiste })
    );

    const platforms = Array.isArray(nastupiste) ? nastupiste : [nastupiste];
    platforms.forEach((platform) => {
      const key = `${zastavka}:${platform}`;
      if (vInfoIntervals.has(key)) {
        clearInterval(vInfoIntervals.get(key));
        vInfoIntervals.delete(key);
        console.log(`  -> stopped vInfo stream for key=${key}`);
      }
    });
  });

  // -------------------------------------------------------------------------
  // Disconnect — clean up all intervals
  // -------------------------------------------------------------------------
  socket.on("disconnect", (reason) => {
    console.log(`[disconnect] socket=${socket.id} reason=${reason}`);
    vInfoIntervals.forEach((_, key) => {
      console.log(`  -> cleared vInfo stream for key=${key}`);
    });
    vInfoIntervals.forEach((interval) => clearInterval(interval));
    vInfoIntervals.clear();
  });

  socket.onAny((eventName, ...args) => {
    console.log(eventName); // 'hello'
    console.log(args); // [ 1, '2', { 3: '4', 5: ArrayBuffer (1) [ 6 ] } ]
  });
});

// ---------------------------------------------------------------------------
// Start listening
// ---------------------------------------------------------------------------
httpServer.listen(PORT, "0.0.0.0", () => {
  console.log(`imhd dummy server listening on http://0.0.0.0:${PORT}`);
  console.log(`Socket.IO path: /rt/sio2`);
});
