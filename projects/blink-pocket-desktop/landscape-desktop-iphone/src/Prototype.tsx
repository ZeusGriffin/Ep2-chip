import { useEffect, useMemo, useRef, useState } from "react";
import {
  ArrowLeftIcon,
  EnterFullScreenIcon,
  GridIcon,
  KeyboardIcon,
  PaperPlaneIcon,
  PlusIcon,
  ReaderIcon,
  Share1Icon,
  SpeakerLoudIcon,
} from "@radix-ui/react-icons";
import { MobileScroll } from "./mobile";

type Message = { id: string; role: "you" | "node"; text: string };
type Conversation = {
  id: string;
  title: string;
  createdAt: number;
  updatedAt: number;
  messages: Message[];
};
type AiState = "checking" | "ready" | "setup" | "error";
type SpeechResult = { 0: { transcript: string } };
type SpeechEvent = { results: ArrayLike<SpeechResult> };
type SpeechRecognitionLike = {
  lang: string;
  continuous: boolean;
  interimResults: boolean;
  start: () => void;
  stop: () => void;
  onresult: ((event: SpeechEvent) => void) | null;
  onend: (() => void) | null;
  onerror: ((event: { error: string }) => void) | null;
};

const STORAGE_KEY = "blink-conversations-v1";
const ACTIVE_KEY = "blink-active-conversation-v1";
const rows = [
  ["Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"],
  ["A", "S", "D", "F", "G", "H", "J", "K", "L"],
  ["Z", "X", "C", "V", "B", "N", "M"],
];

const makeId = () => globalThis.crypto?.randomUUID?.() ?? `${Date.now()}-${Math.random()}`;

const starterMessages: Message[] = [
  { id: "welcome-you", role: "you", text: "Give me a clean landscape workspace where I can read while I type." },
  {
    id: "welcome-node",
    role: "node",
    text: "Desktop view is ready. The reading canvas stays visible while the compact keyboard is open. Ask me anything, use Voice to dictate, or connect a hardware keyboard.",
  },
];

const createConversation = (welcome = false): Conversation => {
  const now = Date.now();
  return {
    id: makeId(),
    title: welcome ? "Landscape desktop" : "Untitled canvas",
    createdAt: now,
    updatedAt: now,
    messages: welcome ? starterMessages : [],
  };
};

const loadConversations = (): Conversation[] => {
  if (typeof window === "undefined") return [createConversation(true)];
  try {
    const saved = JSON.parse(window.localStorage.getItem(STORAGE_KEY) ?? "null") as Conversation[] | null;
    if (Array.isArray(saved) && saved.length && saved.every((item) => item.id && Array.isArray(item.messages))) {
      return saved;
    }
  } catch {
    // A damaged local draft should never prevent BLINK from opening.
  }
  return [createConversation(true)];
};

const titleFrom = (text: string) => {
  const compact = text.replace(/\s+/g, " ").trim();
  return compact.length > 30 ? `${compact.slice(0, 29)}…` : compact;
};

export default function Prototype() {
  const [conversations, setConversations] = useState<Conversation[]>(loadConversations);
  const [activeConversationId, setActiveConversationId] = useState(() => {
    if (typeof window === "undefined") return "";
    return window.localStorage.getItem(ACTIVE_KEY) ?? "";
  });
  const [draft, setDraft] = useState("");
  const [keyboardOpen, setKeyboardOpen] = useState(false);
  const [immersive, setImmersive] = useState(false);
  const [active, setActive] = useState<"canvas" | "read" | "type" | "voice">("read");
  const [pendingConversationId, setPendingConversationId] = useState<string | null>(null);
  const [error, setError] = useState("");
  const [aiState, setAiState] = useState<AiState>("checking");
  const [listening, setListening] = useState(false);
  const recognitionRef = useRef<SpeechRecognitionLike | null>(null);
  const endRef = useRef<HTMLDivElement | null>(null);

  const activeConversation =
    conversations.find((conversation) => conversation.id === activeConversationId) ?? conversations[0];
  const messages = activeConversation?.messages ?? [];
  const isPending = pendingConversationId === activeConversation?.id;

  useEffect(() => {
    if (!activeConversationId && conversations[0]) setActiveConversationId(conversations[0].id);
  }, [activeConversationId, conversations]);

  useEffect(() => {
    window.localStorage.setItem(STORAGE_KEY, JSON.stringify(conversations));
  }, [conversations]);

  useEffect(() => {
    if (activeConversationId) window.localStorage.setItem(ACTIVE_KEY, activeConversationId);
  }, [activeConversationId]);

  useEffect(() => {
    if ("serviceWorker" in navigator) navigator.serviceWorker.register("/sw.js").catch(() => undefined);
    fetch("/api/health", { headers: { accept: "application/json" } })
      .then(async (response) => {
        const data = (await response.json()) as { configured?: boolean };
        setAiState(response.ok && data.configured ? "ready" : "setup");
      })
      .catch(() => setAiState("error"));
  }, []);

  useEffect(() => {
    endRef.current?.scrollIntoView({ behavior: "smooth", block: "end" });
  }, [messages.length, isPending]);

  useEffect(() => {
    if (!keyboardOpen) return;
    const onKeyDown = (event: KeyboardEvent) => {
      if (event.metaKey || event.ctrlKey || event.altKey) return;
      if (event.key === "Backspace") {
        event.preventDefault();
        setDraft((value) => value.slice(0, -1));
      } else if (event.key === "Enter") {
        event.preventDefault();
        void send();
      } else if (event.key === "Escape") {
        setKeyboardOpen(false);
      } else if (event.key.length === 1) {
        event.preventDefault();
        setDraft((value) => `${value}${event.key}`);
      }
    };
    window.addEventListener("keydown", onKeyDown);
    return () => window.removeEventListener("keydown", onKeyDown);
  });

  const wordCount = useMemo(
    () => messages.reduce((total, message) => total + message.text.trim().split(/\s+/).filter(Boolean).length, 0),
    [messages],
  );

  const updateConversation = (id: string, update: (conversation: Conversation) => Conversation) => {
    setConversations((current) => current.map((conversation) => (conversation.id === id ? update(conversation) : conversation)));
  };

  const typeKey = (key: string) => {
    if (key === "⌫") return setDraft((value) => value.slice(0, -1));
    if (key === "SPACE") return setDraft((value) => `${value} `);
    setDraft((value) => `${value}${key.toLowerCase()}`);
  };

  const requestReply = async (conversationId: string, history: Message[]) => {
    setPendingConversationId(conversationId);
    setError("");
    try {
      const response = await fetch("/api/chat", {
        method: "POST",
        headers: { "content-type": "application/json", accept: "application/json" },
        body: JSON.stringify({
          messages: history.slice(-30).map((message) => ({
            role: message.role === "node" ? "assistant" : "user",
            content: message.text,
          })),
        }),
      });
      const data = (await response.json().catch(() => ({}))) as { text?: string; error?: string };
      if (!response.ok || !data.text) throw new Error(data.error || "The AI did not return a response.");
      updateConversation(conversationId, (conversation) => ({
        ...conversation,
        updatedAt: Date.now(),
        messages: [...conversation.messages, { id: makeId(), role: "node", text: data.text! }],
      }));
      setAiState("ready");
    } catch (caught) {
      setError(caught instanceof Error ? caught.message : "The AI connection is unavailable.");
      setAiState("error");
    } finally {
      setPendingConversationId(null);
    }
  };

  const send = async () => {
    const text = draft.trim();
    if (!text || !activeConversation || pendingConversationId) return;
    const userMessage: Message = { id: makeId(), role: "you", text };
    const history = [...activeConversation.messages, userMessage];
    updateConversation(activeConversation.id, (conversation) => ({
      ...conversation,
      title: conversation.messages.some((message) => message.role === "you") ? conversation.title : titleFrom(text),
      updatedAt: Date.now(),
      messages: history,
    }));
    setDraft("");
    await requestReply(activeConversation.id, history);
  };

  const retry = () => {
    if (!activeConversation || pendingConversationId || activeConversation.messages.at(-1)?.role !== "you") return;
    void requestReply(activeConversation.id, activeConversation.messages);
  };

  const chooseMode = (mode: typeof active) => {
    if (mode === "voice") return startVoice();
    setActive(mode);
    if (mode === "type") setKeyboardOpen(true);
    if (mode === "read" || mode === "canvas") setKeyboardOpen(false);
  };

  const startVoice = () => {
    if (listening) {
      recognitionRef.current?.stop();
      return;
    }
    const speechWindow = window as typeof window & {
      SpeechRecognition?: new () => SpeechRecognitionLike;
      webkitSpeechRecognition?: new () => SpeechRecognitionLike;
    };
    const Recognition = speechWindow.SpeechRecognition ?? speechWindow.webkitSpeechRecognition;
    if (!Recognition) {
      setError("Voice dictation is not available in this browser. Type or use a hardware keyboard instead.");
      return;
    }
    const recognition = new Recognition();
    recognition.lang = navigator.language || "en-US";
    recognition.continuous = false;
    recognition.interimResults = false;
    recognition.onresult = (event) => {
      const transcript = Array.from(event.results).map((result) => result[0]?.transcript ?? "").join(" ").trim();
      if (transcript) setDraft((value) => `${value}${value ? " " : ""}${transcript}`);
    };
    recognition.onerror = (event) => setError(`Voice dictation stopped: ${event.error}.`);
    recognition.onend = () => {
      setListening(false);
      setActive("type");
      setKeyboardOpen(true);
    };
    recognitionRef.current = recognition;
    setError("");
    setListening(true);
    setActive("voice");
    setKeyboardOpen(false);
    recognition.start();
  };

  const newCanvas = () => {
    const conversation = createConversation();
    setConversations((current) => [conversation, ...current]);
    setActiveConversationId(conversation.id);
    setDraft("");
    setError("");
    setActive("type");
    setKeyboardOpen(true);
  };

  const selectConversation = (id: string) => {
    setActiveConversationId(id);
    setDraft("");
    setError("");
    setActive("read");
    setKeyboardOpen(false);
  };

  const goBack = () => {
    if (keyboardOpen) {
      setKeyboardOpen(false);
      setActive("read");
    } else {
      setImmersive(false);
    }
  };

  const share = async () => {
    const shareData = { title: "BLINK Desktop", text: activeConversation?.title ?? "Landscape pocket desktop", url: location.href };
    if (navigator.share) await navigator.share(shareData).catch(() => undefined);
    else await navigator.clipboard?.writeText(location.href).catch(() => undefined);
  };

  const statusLabel = isPending
    ? "THINKING"
    : listening
      ? "LISTENING"
      : aiState === "ready"
        ? "AI READY"
        : aiState === "checking"
          ? "CHECKING"
          : aiState === "setup"
            ? "CONNECT AI"
            : "OFFLINE";

  return (
    <div className={`desktop-shell ${immersive ? "is-immersive" : ""}`} data-testid="desktop-shell">
      <header className="top-rail">
        <button className="round-button" type="button" aria-label="Close keyboard or exit immersive view" onClick={goBack}><ArrowLeftIcon /></button>
        <div className="brand-lockup" aria-label="Blink Node desktop">
          <span>BLINK / NODE</span><strong>DESKTOP 01</strong>
        </div>
        <button className="icon-button" type="button" aria-label="Toggle immersive view" aria-pressed={immersive} onClick={() => setImmersive((value) => !value)}>
          <EnterFullScreenIcon />
        </button>
      </header>

      <div className="status-row">
        <img src="/app-assets/reference-orbs.png" alt="Three connected workspace indicators" />
        <span>{wordCount} WORDS</span><span className={`status-live status-${aiState}`}>{statusLabel}</span>
      </div>

      <section className={`workspace ${keyboardOpen ? "keyboard-visible" : ""}`}>
        <aside className="thread-rail" aria-label="Saved conversations">
          <div className="rail-heading">WORKSPACE</div>
          <div className="thread-list">
            {conversations.slice(0, 8).map((conversation, index) => (
              <button
                type="button"
                className={`thread-item ${conversation.id === activeConversation?.id ? "is-active" : ""}`}
                key={conversation.id}
                onClick={() => selectConversation(conversation.id)}
              >
                <span>{String(index + 1).padStart(2, "0")}</span><strong>{conversation.title}</strong>
              </button>
            ))}
          </div>
          <button type="button" className="thread-item new-canvas" onClick={newCanvas}><PlusIcon /><strong>New canvas</strong></button>
        </aside>

        <MobileScroll className="reading-scroll">
          <main className="reading-canvas" aria-label="Conversation reading canvas">
            <div className="document-kicker">ACTIVE CANVAS / CHAT</div>
            <h1>{messages.length ? "More room to think." : "What are we building?"}</h1>
            {!messages.length && <p className="empty-state">Open Type or Voice. Your conversations stay on this device, and the reading canvas remains visible while you write.</p>}
            {messages.map((message) => (
              <article className={`message message-${message.role}`} key={message.id}>
                <span>{message.role === "node" ? "NODE" : "YOU"}</span><p>{message.text}</p>
              </article>
            ))}
            {isPending && (
              <article className="message message-node pending-message" aria-live="polite">
                <span>NODE</span><p><i /><i /><i /><em>Thinking</em></p>
              </article>
            )}
            {error && (
              <div className="connection-notice" role="alert">
                <span>{error}</span>
                {messages.at(-1)?.role === "you" && <button type="button" onClick={retry} disabled={Boolean(pendingConversationId)}>Retry</button>}
              </div>
            )}
            <div className="reading-end" ref={endRef}>END OF CANVAS</div>
          </main>
        </MobileScroll>
      </section>

      <section className={`composer ${keyboardOpen ? "is-open" : ""}`} aria-label="Compact keyboard">
        <div className="draft-line" role="textbox" aria-label="Message draft" aria-live="polite">
          <span>{draft || "Type without covering the page…"}</span>
          <button type="button" className="send-button" onClick={() => void send()} aria-label="Send message" disabled={!draft.trim() || Boolean(pendingConversationId)}><PaperPlaneIcon /></button>
        </div>
        <div className="compact-keys">
          {rows.map((row) => (
            <div className="key-row" key={row.join("")}>
              {row.map((key) => <button type="button" key={key} onClick={() => typeKey(key)}>{key}</button>)}
            </div>
          ))}
          <div className="key-row utility-row">
            <button type="button" onClick={() => typeKey("⌫")}>⌫</button>
            <button type="button" onClick={() => typeKey(",")}>，</button>
            <button type="button" className="space-key" onClick={() => typeKey("SPACE")}>SPACE</button>
            <button type="button" onClick={() => typeKey(".")}>.</button>
            <button type="button" onClick={() => void send()} disabled={!draft.trim() || Boolean(pendingConversationId)}>SEND</button>
            <button type="button" onClick={() => setKeyboardOpen(false)}>HIDE</button>
          </div>
        </div>
      </section>

      <nav className={`floating-dock ${keyboardOpen ? "dock-raised" : ""}`} aria-label="Desktop controls">
        <button type="button" className={active === "canvas" ? "is-active" : ""} onClick={() => chooseMode("canvas")}><GridIcon /><span>Canvas</span></button>
        <button type="button" className={active === "read" ? "is-active" : ""} onClick={() => chooseMode("read")}><ReaderIcon /><span>Read</span></button>
        <button type="button" className={active === "type" ? "is-active" : ""} onClick={() => chooseMode("type")}><KeyboardIcon /><span>Type</span></button>
        <button type="button" className={active === "voice" ? "is-active" : ""} onClick={() => chooseMode("voice")}><SpeakerLoudIcon /><span>{listening ? "Stop" : "Voice"}</span></button>
        <button type="button" onClick={share}><Share1Icon /><span>Share</span></button>
      </nav>
    </div>
  );
}
