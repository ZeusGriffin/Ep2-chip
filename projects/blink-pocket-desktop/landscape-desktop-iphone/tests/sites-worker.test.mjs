import assert from "node:assert/strict";
import { access } from "node:fs/promises";
import test from "node:test";
import worker from "../worker/index.js";

test("serves existing static assets without a fallback", async () => {
  const calls = [];
  const response = await worker.fetch(new Request("https://example.test/assets/app.js"), {
    ASSETS: {
      fetch: async (request) => {
        calls.push(new URL(request.url).pathname);
        return new Response("asset", { status: 200 });
      },
    },
  });

  assert.equal(response.status, 200);
  assert.deepEqual(calls, ["/assets/app.js"]);
});

test("falls back to index.html for an unknown app route", async () => {
  const calls = [];
  const response = await worker.fetch(
    new Request("https://example.test/flow/step-two?source=share", {
      headers: { accept: "text/html" },
    }),
    {
      ASSETS: {
        fetch: async (request) => {
          const url = new URL(request.url);
          calls.push(url.pathname + url.search);
          return new Response(url.pathname === "/index.html" ? "app" : "missing", {
            status: url.pathname === "/index.html" ? 200 : 404,
          });
        },
      },
    },
  );

  assert.equal(response.status, 200);
  assert.deepEqual(calls, ["/flow/step-two?source=share", "/index.html"]);
});

test("does not turn missing API or write requests into the app shell", async () => {
  let calls = 0;
  const env = {
    ASSETS: {
      fetch: async () => {
        calls += 1;
        return new Response("missing", { status: 404 });
      },
    },
  };

  const apiResponse = await worker.fetch(
    new Request("https://example.test/api/missing", { headers: { accept: "application/json" } }),
    env,
  );
  assert.equal(apiResponse.status, 404);
  assert.equal(calls, 0);

  const writeResponse = await worker.fetch(
    new Request("https://example.test/flow", { method: "POST", headers: { accept: "text/html" } }),
    env,
  );
  assert.equal(writeResponse.status, 404);
  assert.equal(calls, 1);
});

test("reports whether the server-side AI secret is configured", async () => {
  const disconnected = await worker.fetch(new Request("https://example.test/api/health"), {});
  assert.deepEqual(await disconnected.json(), { ok: true, configured: false });

  const connected = await worker.fetch(new Request("https://example.test/api/health"), {
    OPENAI_API_KEY: "test-key",
  });
  assert.deepEqual(await connected.json(), { ok: true, configured: true });
});

test("keeps the OpenAI key server-side and returns response text", async () => {
  const originalFetch = globalThis.fetch;
  let upstreamRequest;
  globalThis.fetch = async (request, init) => {
    upstreamRequest = { request, init };
    return Response.json({
      output: [{ content: [{ type: "output_text", text: "Landscape reply" }] }],
    });
  };

  try {
    const response = await worker.fetch(
      new Request("https://example.test/api/chat", {
        method: "POST",
        headers: { "content-type": "application/json" },
        body: JSON.stringify({ messages: [{ role: "user", content: "Hello" }] }),
      }),
      { OPENAI_API_KEY: "server-only-key" },
    );

    assert.equal(response.status, 200);
    assert.deepEqual(await response.json(), { text: "Landscape reply" });
    assert.equal(upstreamRequest.request, "https://api.openai.com/v1/responses");
    assert.equal(upstreamRequest.init.headers.authorization, "Bearer server-only-key");
    assert.equal(JSON.parse(upstreamRequest.init.body).store, false);
  } finally {
    globalThis.fetch = originalFetch;
  }
});

test("emits the files required by Sites packaging", async () => {
  await access(new URL("../dist/client/index.html", import.meta.url));
  await access(new URL("../dist/server/index.js", import.meta.url));
  await access(new URL("../dist/.openai/hosting.json", import.meta.url));
});
