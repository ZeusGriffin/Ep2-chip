// Wuzplay Cyberdeck v8 - WuzSync companion for Scriptable on iPhone.
// Prepares text files for manual transfer to Wuzplay External Storage via MTools BLE.

const fm=FileManager.local();
const base=fm.joinPath(fm.documentsDirectory(),"WuzplayCyberdeck");
const ext=fm.joinPath(base,"External_Storage");
if(!fm.fileExists(base))fm.createDirectory(base); if(!fm.fileExists(ext))fm.createDirectory(ext);
function path(n){return fm.joinPath(ext,n);} function write(n,t){fm.writeString(path(n),String(t).trim()+"\n");}
async function editFile(name,title){const old=fm.fileExists(path(name))?fm.readString(path(name)):"";const a=new Alert();a.title=title;a.message=`Saved as ${name}. Upload this file to Wuzplay External Storage with MTools BLE.`;a.addTextField("Text",old);a.addAction("Save");a.addCancelAction("Cancel");const i=await a.presentAlert();if(i<0)return;write(name,a.textFieldValue(0));}
async function showFolder(){const a=new Alert();a.title="WuzSync";a.message=`Prepared files are stored here:\n${ext}\n\nUse MTools BLE to upload them to the root of Wuzplay External Storage.`;a.addAction("OK");await a.presentAlert();}
(async()=>{const a=new Alert();a.title="WuzSync";["Alerts","Notes","Home","Emergency","Show Export Folder"].forEach(x=>a.addAction(x));a.addCancelAction("Cancel");const i=await a.presentSheet();if(i===0)return editFile("alerts.txt","Alerts");if(i===1)return editFile("notes.txt","Notes");if(i===2)return editFile("home.txt","Home");if(i===3)return editFile("emergency.txt","Emergency");if(i===4)return showFolder();})();
