import { useEffect, useMemo, useState } from "react";
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

type Message = { id: number; role: "you" | "node"; text: string };

const rows = [
  ["Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"],
  ["A", "S", "D", "F", "G", "H", "J", "K", "L"],
  ["Z", "X", "C", "V", "B", "N", "M"],
];

const starterMessages: Message[] = [
  { id: 1, role: "you", text: "Give me a clean landscape workspace where I can read while I type." },
  {
    id: 2,
    role: "node",
    text: "Desktop view is ready. The reading canvas stays visible while the compact keyboard is open. Use the dock to switch between reading, typing, voice, and your app canvas.",
  },
];

export default function Prototype() {
  const [messages, setMessages] = useState<Message[]>(starterMessages);
  const [draft, setDraft] = useState("");
  const [keyboardOpen, setKeyboardOpen] = useState(false);
  const [immersive, setImmersive] = useState(false);
  const [active, setActive] = useState<"canvas" | "read" | "type" | "voice">("read");

  useEffect(() => {
    if ("serviceWorker" in navigator) navigator.serviceWorker.register("/sw.js").catch(() => undefined);
  }, []);

  const wordCount = useMemo(
    () => messages.reduce((total, message) => total + message.text.split(/\s+/).length, 0),
    [messages],
  );

  const typeKey = (key: string) => {
    if (key === "⌫") return setDraft((value) => value.slice(0, -1));
    if (key === "SPACE") return setDraft((value) => `${value} `);
    setDraft((value) => `${value}${key.toLowerCase()}`);
  };

  const send = () => {
    const text = draft.trim();
    if (!text) return;
    setMessages((current) => [...current, { id: Date.now(), role: "you", text }]);
    setDraft("");
  };

  const chooseMode = (mode: typeof active) => {
    setActive(mode);
    if (mode === "type") setKeyboardOpen(true);
    if (mode === "read" || mode === "canvas") setKeyboardOpen(false);
  };

  const share = async () => {
    if (navigator.share) {
      await navigator.share({ title: "BLINK Desktop", text: "Landscape pocket desktop" }).catch(() => undefined);
    }
  };

  return (
    <div className={`desktop-shell ${immersive ? "is-immersive" : ""}`} data-testid="desktop-shell">
      <header className="top-rail">
        <button className="round-button" type="button" aria-label="Back"><ArrowLeftIcon /></button>
        <div className="brand-lockup" aria-label="Blink Node desktop">
          <span>BLINK / NODE</span><strong>DESKTOP 01</strong>
        </div>
        <button className="icon-button" type="button" aria-label="Toggle immersive view" aria-pressed={immersive} onClick={() => setImmersive((value) => !value)}>
          <EnterFullScreenIcon />
        </button>
      </header>

      <div className="status-row">
        <img src="/app-assets/reference-orbs.png" alt="Three connected workspace indicators" />
        <span>{wordCount} WORDS</span><span className="status-live">LIVE</span>
      </div>

      <section className={`workspace ${keyboardOpen ? "keyboard-visible" : ""}`}>
        <aside className="thread-rail" aria-label="Workspace files">
          <div className="rail-heading">WORKSPACE</div>
          <button type="button" className="thread-item is-active"><span>01</span><strong>Landscape desktop</strong></button>
          <button type="button" className="thread-item"><span>02</span><strong>Quick notes</strong></button>
          <button type="button" className="thread-item"><PlusIcon /><strong>New canvas</strong></button>
        </aside>

        <MobileScroll className="reading-scroll">
          <main className="reading-canvas" aria-label="Conversation reading canvas">
            <div className="document-kicker">ACTIVE CANVAS / CHAT</div>
            <h1>More room to think.</h1>
            {messages.map((message) => (
              <article className={`message message-${message.role}`} key={message.id}>
                <span>{message.role === "node" ? "NODE" : "YOU"}</span><p>{message.text}</p>
              </article>
            ))}
            <div className="reading-end">END OF CANVAS</div>
          </main>
        </MobileScroll>
      </section>

      <section className={`composer ${keyboardOpen ? "is-open" : ""}`} aria-label="Compact keyboard">
        <div className="draft-line" role="textbox" aria-label="Message draft" aria-live="polite">
          <span>{draft || "Type without covering the page…"}</span>
          <button type="button" className="send-button" onClick={send} aria-label="Send message" disabled={!draft.trim()}><PaperPlaneIcon /></button>
        </div>
        <div className="compact-keys">
          {rows.map((row) => (
            <div className="key-row" key={row.join("")}>
              {row.map((key) => <button type="button" key={key} onClick={() => typeKey(key)}>{key}</button>)}
            </div>
          ))}
          <div className="key-row utility-row">
            <button type="button" onClick={() => typeKey("⌫")}>⌫</button>
            <button type="button" className="space-key" onClick={() => typeKey("SPACE")}>SPACE</button>
            <button type="button" onClick={send}>SEND</button>
            <button type="button" onClick={() => setKeyboardOpen(false)}>HIDE</button>
          </div>
        </div>
      </section>

      <nav className={`floating-dock ${keyboardOpen ? "dock-raised" : ""}`} aria-label="Desktop controls">
        <button type="button" className={active === "canvas" ? "is-active" : ""} onClick={() => chooseMode("canvas")}><GridIcon /><span>Canvas</span></button>
        <button type="button" className={active === "read" ? "is-active" : ""} onClick={() => chooseMode("read")}><ReaderIcon /><span>Read</span></button>
        <button type="button" className={active === "type" ? "is-active" : ""} onClick={() => chooseMode("type")}><KeyboardIcon /><span>Type</span></button>
        <button type="button" className={active === "voice" ? "is-active" : ""} onClick={() => chooseMode("voice")}><SpeakerLoudIcon /><span>Voice</span></button>
        <button type="button" onClick={share}><Share1Icon /><span>Share</span></button>
      </nav>
    </div>
  );
}
