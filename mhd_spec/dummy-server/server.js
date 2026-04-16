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

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// /**
 // * Build a dummy `tabs` response payload.
 // * Mirrors the shape observed from the real server.
 // *
 // * @param {number} zastavka   - Stop ID sent by the client
 // * @param {number} nastupiste - Platform ID sent by the client
 // * @param {number} now        - Current timestamp in ms
 // */
// function buildTabsPayload(zastavka, nastupiste, now) {
  // return [
    // {
      // zastavka,
      // nastupiste,
      // timestamp: now,
      // nextRowAt: null,
      // tab: [
        // {
          // linka: "30",
          // issi: "1:2548",
          // sekcie: ["ba"],
          // tuZidx: 17,
          // i: -16806570,
          // cas: now + 5 * 60 * 1000,    // arrival in ~5 minutes
          // casCP: now + 3 * 60 * 1000,  // checkpoint time in ~3 minutes
        // },
        // {
          // linka: "31",
          // issi: "1:1099",
          // sekcie: ["ba"],
          // tuZidx: 4,
          // i: -16777216,
          // cas: now + 12 * 60 * 1000,
          // casCP: now + 10 * 60 * 1000,
        // },
      // ],
    // },
  // ];
// }

// /**
 // * Pool of dummy vehicle info objects.
 // * Mirrors the `vInfo` payloads observed from the real server.
 // */
// const DUMMY_VEHICLES = [
  // { id: 10588, lf: 1, ac: 1, img: 705,  imgt: 1, type: "SOR NB 18 City",      issi: "1:1862", operaror: "1" },
  // { id: 10565, lf: 1, ac: 1, img: 1056, imgt: 1, type: "SOR NB 12 City",      issi: "1:2035", operaror: "1" },
  // { id: 10421, lf: 0, ac: 1, img: 312,  imgt: 1, type: "Citaro G",            issi: "1:3011", operaror: "1" },
  // { id: 10302, lf: 1, ac: 0, img: 88,   imgt: 1, type: "Solaris Urbino 12",   issi: "1:0744", operaror: "1" },
// ];

// /** Pick a random vehicle from the pool */
// function randomVehicle() {
  // return DUMMY_VEHICLES[Math.floor(Math.random() * DUMMY_VEHICLES.length)];
// }

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

  // // -------------------------------------------------------------------------
  // // `tabs` event
  // //
  // // Client sends:
  // //   [{ zastavka: 355, nastupiste: 864, timestamp: <ms>, tab: [] }]
  // //
  // // Server responds with a `tabs` event containing the timetable for that
  // // stop/platform combination.
  // // -------------------------------------------------------------------------
  // socket.on("tabs", (payload) => {
    // console.log(`[tabs]     socket=${socket.id}`, JSON.stringify(payload));

    // if (!Array.isArray(payload) || payload.length === 0) {
      // console.warn("  -> invalid tabs payload, ignoring");
      // return;
    // }

    // const now = Date.now();
    // const response = payload
      // .map(({ zastavka, nastupiste }) =>
        // buildTabsPayload(zastavka ?? 355, nastupiste ?? 864, now)
      // )
      // .flat();

    // socket.emit("tabs", response);
    // console.log(`  -> emitted tabs response (${response.length} stop(s))`);
  // });

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
});

// ---------------------------------------------------------------------------
// Start listening
// ---------------------------------------------------------------------------
httpServer.listen(PORT, () => {
  console.log(`imhd dummy server listening on http://localhost:${PORT}`);
  console.log(`Socket.IO path: /rt/sio2`);
});