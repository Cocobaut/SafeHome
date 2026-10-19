#ifndef USER_PAGE_H
#define USER_PAGE_H

#include <Arduino.h>

static const char USER_PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>YOLO UNO Security</title>
<link rel="icon" href="data:,">
<style>
:root{--bg:#0f172a;--card:#1e293b;--line:#334155;--text:#e2e8f0;--muted:#94a3b8;
--ok:#16a34a;--idle:#475569;--bad:#dc2626;--warn:#d97706;--accent:#2563eb}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif}
.wrap{max-width:720px;margin:0 auto;padding:16px}
h1{font-size:20px;margin:4px 0 16px;display:flex;justify-content:space-between;align-items:center}
.dot{width:10px;height:10px;border-radius:50%;background:var(--bad);display:inline-block}
.dot.on{background:var(--ok)}
.lock{border-radius:14px;padding:20px;text-align:center;margin-bottom:16px;background:var(--ok);transition:background .3s}
.lock.open{background:var(--warn)}
.lock .big{font-size:28px;font-weight:800;letter-spacing:1px}
.lock .sub{margin-top:6px;font-size:15px;opacity:.95;min-height:20px}
.banner{display:none;background:var(--bad);border-radius:12px;padding:12px;margin-bottom:16px;font-weight:700;text-align:center;animation:blink 1s infinite}
.banner.show{display:block}
@keyframes blink{50%{opacity:.6}}
.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:12px;margin-bottom:16px}
.card{border-radius:14px;padding:14px;background:var(--idle);transition:background .3s;min-height:110px}
.card.ok{background:var(--ok)}
.card.bad{background:var(--bad);box-shadow:0 0 0 3px #fca5a5 inset}
.card .name{font-size:13px;opacity:.85}
.card .state{font-size:17px;font-weight:700;margin:6px 0}
.card .val{font-size:13px;opacity:.9}
.panel{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:16px}
.panel h2{font-size:16px;margin:0 0 12px}
form{display:flex;gap:8px}
input{flex:1;min-width:0;padding:12px;border-radius:10px;border:1px solid var(--line);background:#0b1220;color:var(--text);font-size:16px}
button{padding:12px 18px;border:0;border-radius:10px;background:var(--accent);color:#fff;font-weight:700;font-size:16px;cursor:pointer}
button:disabled{opacity:.5}
.msg{margin-top:10px;min-height:20px;font-size:14px}
.msg.err{color:#fca5a5}.msg.ok{color:#86efac}
.foot{margin-top:16px;text-align:center;font-size:13px;color:var(--muted)}
.foot a{color:var(--muted)}
@media(max-width:420px){.grid{grid-template-columns:1fr}}
</style>
</head>
<body>
<div class="wrap">
  <h1>YOLO UNO Security <span class="dot" id="conn" title="Kết nối"></span></h1>

  <div class="banner" id="gasBanner">⚠ PHÁT HIỆN RÒ RỈ GAS — ĐÃ MỞ CHỐT THOÁT HIỂM</div>

  <div class="lock" id="lock">
    <div class="big" id="lockText">ĐANG ĐÓNG</div>
    <div class="sub" id="lockSub">Hệ thống chốt cửa đang khóa</div>
  </div>

  <div class="grid">
    <div class="card" id="c_us1"><div class="name">Ultrasonic 1 · Ban công trước</div><div class="state">--</div><div class="val">--</div></div>
    <div class="card" id="c_us2"><div class="name">Ultrasonic 2 · Ban công sau</div><div class="state">--</div><div class="val">--</div></div>
    <div class="card" id="c_door"><div class="name">Cảm biến cửa</div><div class="state">--</div><div class="val">--</div></div>
    <div class="card" id="c_gas"><div class="name">Cảm biến khí Gas</div><div class="state">--</div><div class="val">--</div></div>
  </div>

  <div class="panel">
    <h2>Mở cửa bằng mật khẩu</h2>
    <form id="f">
      <input type="password" id="pw" placeholder="Nhập mật khẩu" autocomplete="off" inputmode="numeric" required>
      <button id="btn">Mở cửa</button>
    </form>
    <div class="msg" id="msg"></div>
  </div>

  <div class="foot"><a href="/admin">Trang quản trị</a></div>
</div>

<script>
const $=id=>document.getElementById(id);

function setCard(id,cls,state,val){
  const c=$(id);c.className='card '+cls;
  c.querySelector('.state').textContent=state;
  c.querySelector('.val').textContent=val;
}
function cm(v){return v>=400?'ngoài tầm':v.toFixed(0)+' cm'}

function render(s){
  const open=!s.locked;
  $('lock').className='lock'+(open?' open':'');
  $('lockText').textContent=open?'ĐANG MỞ':'ĐANG ĐÓNG';
  if(s.unlock>0) $('lockSub').textContent='Tự động khóa lại sau '+Math.ceil(s.unlock/1000)+' giây';
  else if(s.gas.a) $('lockSub').textContent='Mở khẩn cấp do rò rỉ gas';
  else $('lockSub').textContent=open?'Một số chốt đang mở':'Hệ thống chốt cửa đang khóa';
  $('gasBanner').className='banner'+(s.gas.a?' show':'');

  s.us.forEach((u,i)=>{
    const id='c_us'+(i+1),val='Đo: '+cm(u.d)+' · Mốc: '+cm(u.b);
    if(s.cal) setCard(id,'','Đang hiệu chuẩn...',val);
    else if(s.sup) setCard(id,'','Tạm tắt (mở cửa)',val);
    else if(u.a) setCard(id,'bad','PHÁT HIỆN XÂM NHẬP',val);
    else setCard(id,'ok','Bình thường',val);
  });

  const dv=s.door.open?'Cửa đang mở':'Cửa đang đóng';
  if(s.door.a) setCard('c_door','bad','CỬA BỊ MỞ TRÁI PHÉP',dv);
  else setCard('c_door',s.door.open?'':'ok',s.door.open?'Đang mở (hợp lệ)':'Bình thường',dv);

  const gv='Giá trị: '+s.gas.v+' / ngưỡng '+s.gas.th;
  if(s.gas.a) setCard('c_gas','bad','RÒ RỈ GAS',gv);
  else if(s.gas.warm) setCard('c_gas','','Đang làm nóng...',gv);
  else setCard('c_gas','ok','Bình thường',gv);
}

async function poll(){
  try{
    const ctl=new AbortController(),t=setTimeout(()=>ctl.abort(),2500);
    const r=await fetch('/status',{cache:'no-store',signal:ctl.signal});
    clearTimeout(t);
    render(await r.json());
    $('conn').className='dot on';
  }catch(e){$('conn').className='dot'}
  setTimeout(poll,1000);
}

$('f').addEventListener('submit',async e=>{
  e.preventDefault();
  const msg=$('msg');$('btn').disabled=true;
  try{
    const r=await fetch('/unlock',{method:'POST',body:new URLSearchParams({password:$('pw').value})});
    const j=await r.json();
    if(j.ok){msg.className='msg ok';msg.textContent='Đã mở cửa trong 10 giây';}
    else if(j.err==='locked'){msg.className='msg err';msg.textContent='Sai quá nhiều lần, thử lại sau '+j.wait+' giây';}
    else{msg.className='msg err';msg.textContent='Sai mật khẩu';}
  }catch(e){msg.className='msg err';msg.textContent='Mất kết nối thiết bị';}
  $('pw').value='';$('btn').disabled=false;
});

poll();
</script>
</body>
</html>
)rawliteral";

#endif
