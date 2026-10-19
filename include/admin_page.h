#ifndef ADMIN_PAGE_H
#define ADMIN_PAGE_H

#include <Arduino.h>

static const char ADMIN_PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Admin · YOLO UNO</title>
<link rel="icon" href="data:,">
<style>
:root{--bg:#0f172a;--card:#1e293b;--line:#334155;--text:#e2e8f0;--muted:#94a3b8;
--ok:#16a34a;--bad:#dc2626;--warn:#d97706;--accent:#2563eb;--gray:#475569}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif}
.wrap{max-width:760px;margin:0 auto;padding:16px}
h1{font-size:20px;margin:4px 0 16px}
.panel{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:16px;margin-bottom:14px}
.panel h2{font-size:15px;margin:0 0 12px;color:var(--muted);text-transform:uppercase;letter-spacing:.5px}
.hidden{display:none}
form{display:flex;gap:8px}
input{flex:1;min-width:0;padding:12px;border-radius:10px;border:1px solid var(--line);background:#0b1220;color:var(--text);font-size:16px}
button{padding:10px 16px;border:0;border-radius:10px;background:var(--accent);color:#fff;font-weight:700;font-size:15px;cursor:pointer}
button.on{background:var(--bad)}button.off{background:var(--gray)}button.go{background:var(--ok)}
button:disabled{opacity:.5}
.msg{margin-top:10px;min-height:20px;font-size:14px;color:#fca5a5}
.top{display:flex;justify-content:space-between;align-items:center;gap:10px;flex-wrap:wrap}
.sess{font-variant-numeric:tabular-nums;font-weight:700}
.sess.low{color:#fca5a5}
.row{display:flex;justify-content:space-between;align-items:center;gap:10px;padding:10px 0;border-top:1px solid var(--line)}
.row:first-of-type{border-top:0}
.row .btns{display:flex;gap:6px}
.badge{display:inline-block;padding:3px 10px;border-radius:999px;font-size:13px;font-weight:700;background:var(--gray)}
.badge.ok{background:var(--ok)}.badge.bad{background:var(--bad)}.badge.warn{background:var(--warn)}
.gas{font-size:40px;font-weight:800;font-variant-numeric:tabular-nums}
.bar{height:12px;background:#0b1220;border-radius:6px;overflow:hidden;position:relative;margin-top:8px}
.bar i{display:block;height:100%;background:var(--ok);width:0;transition:width .3s}
.bar i.bad{background:var(--bad)}
.bar b{position:absolute;top:0;bottom:0;width:2px;background:#fff}
.small{font-size:13px;color:var(--muted)}
</style>
</head>
<body>
<div class="wrap">
  <h1>Quản trị · YOLO UNO Security</h1>

  <div class="panel" id="login">
    <h2>Đăng nhập Admin</h2>
    <form id="lf">
      <input type="password" id="apw" placeholder="Mật khẩu Admin" autocomplete="current-password" required>
      <button id="lbtn">Đăng nhập</button>
    </form>
    <div class="msg" id="lmsg"></div>
  </div>

  <div id="dash" class="hidden">
    <div class="panel top">
      <div>Phiên còn lại: <span class="sess" id="sess">10:00</span></div>
      <button class="off" id="logout">Đăng xuất</button>
    </div>

    <div class="panel">
      <h2>Chốt cửa (Servo)</h2>
      <div id="servos"></div>
    </div>

    <div class="panel">
      <h2>Thiết bị cảnh báo</h2>
      <div class="row"><div>Còi báo động <span class="badge" id="bz">--</span></div>
        <div class="btns"><button class="on" data-dev="buzzer" data-state="on">BẬT</button><button class="off" data-dev="buzzer" data-state="off">TẮT</button></div></div>
      <div class="row"><div>Relay xịt cay <span class="badge" id="rl">--</span></div>
        <div class="btns"><button class="on" data-dev="relay" data-state="on">BẬT</button><button class="off" data-dev="relay" data-state="off">TẮT</button></div></div>
      <div class="small">TẮT còi cũng tắt cảnh báo cửa đã chốt. Còi báo gas vẫn kêu cho tới khi hết gas.</div>
    </div>

    <div class="panel">
      <h2>Cảm biến khí Gas (Analog)</h2>
      <div class="top"><div class="gas" id="gasRaw">--</div><span class="badge" id="gasSt">--</span></div>
      <div class="bar"><i id="gasBar"></i><b id="gasTh"></b></div>
      <div class="small" id="gasInfo">--</div>
    </div>

    <div class="panel">
      <h2>Cảm biến khác</h2>
      <div class="row"><div>Ultrasonic 1 <span class="small" id="u1">--</span></div><span class="badge" id="u1s">--</span></div>
      <div class="row"><div>Ultrasonic 2 <span class="small" id="u2">--</span></div><span class="badge" id="u2s">--</span></div>
      <div class="row"><div>Cảm biến cửa</div><span class="badge" id="dr">--</span></div>
      <div class="row"><div class="btns"><button class="off" id="cal">Hiệu chuẩn siêu âm</button><button class="go" id="rst">Reset cảnh báo</button></div></div>
    </div>
  </div>
</div>

<script>
const $=id=>document.getElementById(id);
let token=sessionStorage.getItem('tk')||'',expireAt=0,timer=null;

function badge(id,cls,text){const b=$(id);b.className='badge '+cls;b.textContent=text;}
function cm(v){return v>=400?'ngoài tầm':v.toFixed(0)+' cm'}

(function buildServos(){
  let h='';
  for(let i=1;i<=3;i++){
    h+='<div class="row"><div>Servo '+i+' <span class="badge" id="sv'+i+'">--</span> <span class="small" id="sa'+i+'"></span></div>'+
       '<div class="btns"><button class="go" data-servo="'+i+'" data-action="open">Mở</button>'+
       '<button class="off" data-servo="'+i+'" data-action="close">Đóng</button></div></div>';
  }
  $('servos').innerHTML=h;
})();

function showLogin(text){
  token='';sessionStorage.removeItem('tk');
  $('dash').classList.add('hidden');$('login').classList.remove('hidden');
  $('lmsg').textContent=text||'';
  clearTimeout(timer);
}
function showDash(){
  $('login').classList.add('hidden');$('dash').classList.remove('hidden');
}

async function api(path,data){
  const body=new URLSearchParams(Object.assign({token:token},data||{}));
  const r=await fetch(path,{method:'POST',body:body,cache:'no-store'});
  if(r.status===401){showLogin('Phiên đăng nhập đã hết hạn (10 phút). Vui lòng đăng nhập lại.');throw new Error('401');}
  return r.json();
}

function render(s){
  expireAt=Date.now()+s.session;
  for(let i=0;i<3;i++){
    s.servo[i]?badge('sv'+(i+1),'warn','ĐANG MỞ'):badge('sv'+(i+1),'ok','ĐANG ĐÓNG');
    $('sa'+(i+1)).textContent=s.mv&&s.mv[i]?'đang quay...':'';
  }
  s.buzzer?badge('bz','bad','ĐANG BẬT'):badge('bz','','TẮT');
  s.relay?badge('rl','bad','ĐANG BẬT'):badge('rl','','TẮT');

  $('gasRaw').textContent=s.raw;
  const pct=Math.min(100,s.raw/4095*100);
  $('gasBar').style.width=pct+'%';$('gasBar').className=s.gas.a?'bad':'';
  $('gasTh').style.left=(s.gas.th/4095*100)+'%';
  $('gasInfo').textContent='Đã lọc: '+s.gas.v+' · Ngưỡng: '+s.gas.th+' (0–4095)';
  if(s.gas.a) badge('gasSt','bad','RÒ RỈ GAS');
  else if(s.gas.warm) badge('gasSt','','Đang làm nóng');
  else badge('gasSt','ok','Bình thường');

  s.us.forEach((u,i)=>{
    $('u'+(i+1)).textContent=cm(u.d)+' / mốc '+cm(u.b);
    if(s.cal) badge('u'+(i+1)+'s','','Hiệu chuẩn');
    else if(s.sup) badge('u'+(i+1)+'s','','Tạm tắt');
    else if(u.a) badge('u'+(i+1)+'s','bad','XÂM NHẬP');
    else badge('u'+(i+1)+'s','ok','Bình thường');
  });
  if(s.door.a) badge('dr','bad','MỞ TRÁI PHÉP');
  else s.door.open?badge('dr','warn','Đang mở'):badge('dr','ok','Đang đóng');
}

function tickSession(){
  const left=Math.max(0,expireAt-Date.now());
  const m=Math.floor(left/60000),sec=Math.floor(left/1000)%60;
  const el=$('sess');el.textContent=m+':'+String(sec).padStart(2,'0');
  el.className='sess'+(left<60000?' low':'');
  if(token&&expireAt&&left<=0) showLogin('Phiên đăng nhập đã hết hạn (10 phút). Vui lòng đăng nhập lại.');
}
setInterval(tickSession,1000);

async function poll(){
  if(!token) return;
  try{render(await api('/admin/status'));}catch(e){if(!token) return;}
  timer=setTimeout(poll,1000);
}

function start(){showDash();clearTimeout(timer);poll();}

$('lf').addEventListener('submit',async e=>{
  e.preventDefault();$('lbtn').disabled=true;$('lmsg').textContent='';
  try{
    const r=await fetch('/admin/login',{method:'POST',body:new URLSearchParams({password:$('apw').value})});
    const j=await r.json();
    if(j.ok){token=j.token;sessionStorage.setItem('tk',token);expireAt=Date.now()+j.session;start();}
    else if(j.err==='locked') $('lmsg').textContent='Sai quá nhiều lần, thử lại sau '+j.wait+' giây';
    else $('lmsg').textContent='Sai mật khẩu Admin';
  }catch(e){$('lmsg').textContent='Mất kết nối thiết bị';}
  $('apw').value='';$('lbtn').disabled=false;
});

$('logout').addEventListener('click',async()=>{
  try{await api('/admin/logout');}catch(e){}
  showLogin('Đã đăng xuất');
});

document.addEventListener('click',async e=>{
  const b=e.target.closest('button');if(!b||!token) return;
  try{
    if(b.dataset.servo) await api('/admin/servo',{id:b.dataset.servo,action:b.dataset.action});
    else if(b.dataset.dev) await api('/admin/'+b.dataset.dev,{state:b.dataset.state});
    else if(b.id==='cal') await api('/admin/calibrate');
    else if(b.id==='rst') await api('/admin/reset');
    else return;
    render(await api('/admin/status'));
  }catch(e){}
});

if(token) start();
</script>
</body>
</html>
)rawliteral";

#endif
