const OPENAI_RESPONSES_URL = "https://api.openai.com/v1/responses";
const MAX_REQUEST_BYTES = 100_000;
const MAX_MESSAGES = 30;
const MAX_MESSAGE_CHARS = 20_000;

const json = (body, status = 200) =>
  new Response(JSON.stringify(body), {
    status,
    headers: {
      "content-type": "application/json; charset=utf-8",
      "cache-control": "no-store",
      "x-content-type-options": "nosniff",
    },
  });

const extractResponseText = (data) => {
  if (typeof data?.output_text === "string" && data.output_text.trim()) return data.output_text.trim();
  if (!Array.isArray(data?.output)) return "";
  return data.output
    .flatMap((item) => (Array.isArray(item?.content) ? item.content : []))
    .map((part) => (typeof part?.text === "string" ? part.text : ""))
    .filter(Boolean)
    .join("\n")
    .trim();
};

const parseMessages = (value) => {
  if (!Array.isArray(value) || value.length === 0 || value.length > MAX_MESSAGES) return null;
  const messages = [];
  for (const item of value) {
    if (!item || !["user", "assistant"].includes(item.role) || typeof item.content !== "string") return null;
    const content = item.content.trim();
    if (!content || content.length > MAX_MESSAGE_CHARS) return null;
    messages.push({ role: item.role, content });
  }
  return messages;
};

const handleChat = async (request, env) => {
  if (request.method !== "POST") return json({ error: "Method not allowed." }, 405);
  if (!env.OPENAI_API_KEY) {
    return json({ error: "AI connection is not finished yet. Connect OpenAI to activate replies." }, 503);
  }

  const rawBody = await request.text();
  if (rawBody.length > MAX_REQUEST_BYTES) return json({ error: "Conversation is too large." }, 413);

  let body;
  try {
    body = JSON.parse(rawBody);
  } catch {
    return json({ error: "Request body must be valid JSON." }, 400);
  }
  const messages = parseMessages(body?.messages);
  if (!messages) return json({ error: "Send 1–30 valid conversation messages." }, 400);

  let openAIResponse;
  try {
    openAIResponse = await fetch(OPENAI_RESPONSES_URL, {
      method: "POST",
      headers: {
        authorization: `Bearer ${env.OPENAI_API_KEY}`,
        "content-type": "application/json",
      },
      body: JSON.stringify({
        model: env.OPENAI_MODEL || "gpt-6-astra",
        instructions:
          "You are NODE, the concise AI inside BLINK Landscape Desktop. Give direct, useful answers formatted for a narrow reading canvas. Preserve important detail, but avoid unnecessary preambles.",
        input: messages,
        max_output_tokens: 1400,
        store: false,
      }),
    });
  } catch {
    return json({ error: "OpenAI could not be reached. Try again." }, 502);
  }

  const data = await openAIResponse.json().catch(() => null);
  if (!openAIResponse.ok) {
    const upstreamMessage = typeof data?.error?.message === "string" ? data.error.message : "OpenAI rejected the request.";
    return json({ error: upstreamMessage }, openAIResponse.status >= 500 ? 502 : 400);
  }

  const text = extractResponseText(data);
  if (!text) return json({ error: "OpenAI returned an empty response." }, 502);
  return json({ text });
};

export default {
  async fetch(request, env) {
    const url = new URL(request.url);

    if (url.pathname === "/api/health") {
      if (request.method !== "GET" && request.method !== "HEAD") return json({ error: "Method not allowed." }, 405);
      return json({ ok: true, configured: Boolean(env.OPENAI_API_KEY) });
    }
    if (url.pathname === "/api/chat") return handleChat(request, env);
    if (url.pathname.startsWith("/api/")) return json({ error: "Not found." }, 404);

    const response = await env.ASSETS.fetch(request);
    const acceptsHtml = request.headers.get("accept")?.includes("text/html");

    if (response.status !== 404 || !acceptsHtml || !["GET", "HEAD"].includes(request.method)) {
      return response;
    }

    const indexUrl = new URL(request.url);
    indexUrl.pathname = "/index.html";
    indexUrl.search = "";
    return env.ASSETS.fetch(new Request(indexUrl, request));
  },
};
