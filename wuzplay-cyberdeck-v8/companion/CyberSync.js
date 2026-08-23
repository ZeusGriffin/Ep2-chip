// Wuzplay Cyberdeck v8 - CyberSync companion for Scriptable on iPhone.
// Phone-assisted only: this script does not modify DFU firmware.

const fm = FileManager.local();
const base = fm.joinPath(fm.documentsDirectory(), "WuzplayCyberdeck");
const ext = fm.joinPath(base, "External_Storage");
if (!fm.fileExists(base)) fm.createDirectory(base);
if (!fm.fileExists(ext)) fm.createDirectory(ext);

function writeText(name, text) { fm.writeString(fm.joinPath(ext, name), String(text).trim() + "\n"); }
function readText(name, fallback = "") { const p = fm.joinPath(ext, name); return fm.fileExists(p) ? fm.readString(p) : fallback; }
function now() { return new Date().toISOString(); }
function utf8Bytes(s) { return Array.from(unescape(encodeURIComponent(s))).map(c => c.charCodeAt(0)); }
function hex(s) { return utf8Bytes(s).map(b => b.toString(16).padStart(2, "0")).join(""); }
function rotr(n, x) { return (x >>> n) | (x << (32 - n)); }
function sha256(text) {
  const K=[0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2];
  const H=[0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19];
  const bytes=utf8Bytes(text),bitLen=bytes.length*8; bytes.push(0x80); while((bytes.length%64)!==56)bytes.push(0);
  const hi=Math.floor(bitLen/0x100000000),lo=bitLen>>>0; for(let i=3;i>=0;i--)bytes.push((hi>>>(i*8))&255); for(let i=3;i>=0;i--)bytes.push((lo>>>(i*8))&255);
  for(let off=0;off<bytes.length;off+=64){const w=new Array(64);for(let i=0;i<16;i++)w[i]=((bytes[off+4*i]<<24)|(bytes[off+4*i+1]<<16)|(bytes[off+4*i+2]<<8)|bytes[off+4*i+3])>>>0;for(let i=16;i<64;i++){const s0=(rotr(7,w[i-15])^rotr(18,w[i-15])^(w[i-15]>>>3))>>>0;const s1=(rotr(17,w[i-2])^rotr(19,w[i-2])^(w[i-2]>>>10))>>>0;w[i]=(w[i-16]+s0+w[i-7]+s1)>>>0;}let[a,b,c,d,e,f,g,h]=H;for(let i=0;i<64;i++){const S1=(rotr(6,e)^rotr(11,e)^rotr(25,e))>>>0;const ch=((e&f)^((~e)&g))>>>0;const t1=(h+S1+ch+K[i]+w[i])>>>0;const S0=(rotr(2,a)^rotr(13,a)^rotr(22,a))>>>0;const maj=((a&b)^(a&c)^(b&c))>>>0;const t2=(S0+maj)>>>0;h=g;g=f;f=e;e=(d+t1)>>>0;d=c;c=b;b=a;a=(t1+t2)>>>0;}H[0]=(H[0]+a)>>>0;H[1]=(H[1]+b)>>>0;H[2]=(H[2]+c)>>>0;H[3]=(H[3]+d)>>>0;H[4]=(H[4]+e)>>>0;H[5]=(H[5]+f)>>>0;H[6]=(H[6]+g)>>>0;H[7]=(H[7]+h)>>>0;}
  return H.map(x=>x.toString(16).padStart(8,"0")).join("");
}
function uuid(){const b=Array.from({length:16},()=>Math.floor(Math.random()*256));b[6]=(b[6]&15)|64;b[8]=(b[8]&63)|128;const h=b.map(x=>x.toString(16).padStart(2,"0")).join("");return `${h.slice(0,8)}-${h.slice(8,12)}-${h.slice(12,16)}-${h.slice(16,20)}-${h.slice(20)}`;}
async function show(title,text){const a=new Alert();a.title=title;a.message=text;a.addAction("OK");await a.presentAlert();}
async function prompt(title,message=""){const a=new Alert();a.title=title;a.message=message;a.addTextField("Text");a.addAction("Run");a.addCancelAction("Cancel");const i=await a.presentAlert();return i<0?null:a.textFieldValue(0);}
async function systemMode(){const battery=Math.round(Device.batteryLevel()*100);const s=[`CyberSync System`,`Generated: ${now()}`,`Device: ${Device.name()}`,`Model: ${Device.model()}`,`OS: ${Device.systemName()} ${Device.systemVersion()}`,`Battery: ${battery}%${Device.isCharging()?" (charging)":""}`,`Locale: ${Device.locale()}`].join("\n");writeText("system.txt",s);await show("CyberSync System",s);}
async function networkMode(){let publicIP="Unavailable";try{publicIP=(await new Request("https://api.ipify.org").loadString()).trim();}catch(_){}const s=[`CyberSync Network`,`Generated: ${now()}`,`Public IP: ${publicIP}`].join("\n");writeText("network.txt",s);await show("CyberSync Network",s);}
async function alertsMode(){await show("CyberSync Alerts",readText("alerts.txt","No saved alerts. Share alert text into WuzSync/CyberSync first."));}
async function dashboardMode(){const s=[`WUZPLAY CYBERDECK`,`Updated: ${now()}`,``,`SYSTEM`,readText("system.txt","Run CyberSync System first.").trim(),``,`NETWORK`,readText("network.txt","Run CyberSync Network first.").trim(),``,`ALERTS`,readText("alerts.txt","No saved alerts.").trim()].join("\n");writeText("cyber.txt",s);await show("Cyber Dashboard",s);}
async function toolsMode(){const a=new Alert();a.title="CyberSync Tools";["UUID","Base64","Hex","SHA-256"].forEach(x=>a.addAction(x));a.addCancelAction("Cancel");const choice=await a.presentSheet();if(choice<0)return;if(choice===0){const out=uuid();Pasteboard.copyString(out);return show("UUID",out+"\n\nCopied to clipboard.");}const input=await prompt(["","Base64","Hex","SHA-256"][choice]||"Tool");if(input===null)return;let out="";if(choice===1)out=Data.fromString(input).toBase64String();if(choice===2)out=hex(input);if(choice===3)out=sha256(input);Pasteboard.copyString(out);await show("Result",out+"\n\nCopied to clipboard.");}
async function allMode(){await systemMode();await networkMode();await dashboardMode();}
(async()=>{const mode=(args.queryParameters.mode||"dashboard").toLowerCase();if(mode==="system")return systemMode();if(mode==="network")return networkMode();if(mode==="tools")return toolsMode();if(mode==="alerts")return alertsMode();if(mode==="all")return allMode();return dashboardMode();})();
