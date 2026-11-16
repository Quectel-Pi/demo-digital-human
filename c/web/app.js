import * as THREE from 'three';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
import { VRMLoaderPlugin, VRMUtils } from '@pixiv/three-vrm';

const $ = id => document.getElementById(id);
function setText(id,text){const el=$(id);if(el.textContent!==text)el.textContent=text;}
const renderer = new THREE.WebGLRenderer({canvas:$('avatar'),alpha:true,antialias:false,powerPreference:'high-performance'});
renderer.setPixelRatio(0.85);
renderer.outputColorSpace = THREE.SRGBColorSpace;
renderer.toneMapping = THREE.ACESFilmicToneMapping;
renderer.toneMappingExposure = 1.0;
const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(29,1,0.1,20);
camera.position.set(0,1.34,1.95);
camera.lookAt(0,1.28,0);
scene.add(new THREE.HemisphereLight(0xd8f3ff,0x59607c,1.0));
const key = new THREE.DirectionalLight(0xfff4ea,1.3); key.position.set(-1,2,3); scene.add(key);
const rim = new THREE.DirectionalLight(0x71dfff,0.7); rim.position.set(2,1,-1); scene.add(rim);
let vrm, state = {state:'idle',openness:0,wide:0,round:0,mic:0}, lastReceived=0;
let preview=false, time=0, blinkAt=2, blinkStart=-100, lookNext=0;
let frames=0, sampleStart=performance.now(), frameTimes=[], previous=performance.now();
let lastUi=0;
const smooth = {aa:0,ih:0,ou:0,ee:0,oh:0};
const gaze = new THREE.Object3D();gaze.position.set(0,1.35,3);scene.add(gaze);
const bars=Array.from({length:28},()=>{const el=document.createElement('i');$('wave').append(el);return el;});
const stateNames={idle:'等待你开口',listen:'正在听你说',think:'正在思考',speak:'正在回答'};
// Recolour licensed model textures once, preserving their alpha and highlights.
// This changes only appearance; facial animation remains actual mesh morphing.
const recoloured=new Map();
function coolTexture(source,iris=false){
  if(!source?.image)return source;
  const cacheKey=source.uuid+iris;if(recoloured.has(cacheKey))return recoloured.get(cacheKey);
  const canvas=document.createElement('canvas'),image=source.image;
  const scale=Math.min(1,1024/Math.max(image.width,image.height));
  canvas.width=Math.round(image.width*scale);canvas.height=Math.round(image.height*scale);
  const ctx=canvas.getContext('2d',{willReadFrequently:true});ctx.drawImage(image,0,0,canvas.width,canvas.height);
  const pixels=ctx.getImageData(0,0,canvas.width,canvas.height);
  for(let i=0;i<pixels.data.length;i+=4){
    const luminance=(pixels.data[i]*.3+pixels.data[i+1]*.59+pixels.data[i+2]*.11)/255;
    const v=iris?luminance:Math.pow(luminance,.63);
    pixels.data[i]=255*v*(iris ? 0.30 : 0.73);pixels.data[i+1]=255*v*(iris ? 0.72 : 0.87);pixels.data[i+2]=255*v;
  }
  ctx.putImageData(pixels,0,0);
  const texture=source.clone();texture.image=canvas;texture.needsUpdate=true;
  recoloured.set(cacheKey,texture);return texture;
}
const loader = new GLTFLoader(); loader.register(parser=>new VRMLoaderPlugin(parser));
loader.load('./model.vrm', gltf=>{
  vrm=gltf.userData.vrm;
  VRMUtils.removeUnnecessaryVertices(gltf.scene);VRMUtils.combineSkeletons(gltf.scene);VRMUtils.combineMorphs(vrm);
  // Skip the second outline pass on the embedded GPU, keeping the skinned mesh.
  vrm.scene.traverse(obj=>{if(obj.material){for(const mat of (Array.isArray(obj.material)?obj.material:[obj.material])){
    if(mat.isOutline){mat.visible=false;continue;}
    if(mat.name.includes('_HAIR')){
      mat.map=coolTexture(mat.map);if(mat.shadeMultiplyTexture)mat.shadeMultiplyTexture=coolTexture(mat.shadeMultiplyTexture);
      mat.color?.set('#e3efff');mat.shadeColorFactor?.set('#97b8e4');
    }
    if(mat.name.includes('EyeIris'))mat.map=coolTexture(mat.map,true);
    if(mat.name.includes('Tops_')){mat.color?.set('#365d82');mat.shadeColorFactor?.set('#233e60');}
    mat.needsUpdate=true;
  }}});
  VRMUtils.rotateVRM0(vrm);
  scene.add(vrm.scene);
  vrm.lookAt.target=gaze;
  // Rest pose is T-pose; lower the arms using normalized humanoid bones.
  bone('leftUpperArm').rotation.z=-1.42;
  bone('rightUpperArm').rotation.z=1.42;
  bone('leftLowerArm').rotation.y=-0.16;
  bone('rightLowerArm').rotation.y=0.16;
  $('caption').textContent='你好，我是曼波。';
  window.avatarReady=true;
},undefined,error=>{$('caption').textContent='3D 模型加载失败：'+error.message;console.error(error);});
function bone(name){return vrm.humanoid.getNormalizedBoneNode(name) || new THREE.Object3D();}
function resize(){const el=$('stage'); renderer.setSize(el.clientWidth,el.clientHeight,false);camera.aspect=el.clientWidth/el.clientHeight;camera.updateProjectionMatrix();}
new ResizeObserver(resize).observe($('stage')); resize();
$('preview').onclick=()=>{preview=!preview;$('preview').textContent=preview?'结束无声预览':'无声动作预览';};
// Poll sequentially: never accumulate requests when the board is busy.
async function poll(){
  try {const response=await fetch('./api/state',{cache:'no-store',signal:AbortSignal.timeout(1800)});
    if(!response.ok)throw Error(response.status);state=await response.json();lastReceived=performance.now();
    setText('question',state.question||'向我打个招呼吧');
    setText('answer',state.answer||'我在这里，准备好与你交流。');
    setText('caption',state.answer?state.answer.slice(-90):state.state==='listen'?'我在听，你说。':'你好，我是曼波。');
  }catch{ /* Keep the model alive, but decay the mouth on a stale connection. */ }
  setTimeout(poll,40);
}poll();
function animate(now){
  requestAnimationFrame(animate);
  const rawDelta=(now-previous)/1000, dt=Math.min(rawDelta,0.08);previous=now;time+=dt;
  const connected=now-lastReceived<1200;
  const speaking=preview || (connected&&state.state==='speak');
  const open=preview?Math.max(0,Math.sin(time*11)*0.45+Math.sin(time*17)*0.2+0.25):(speaking?THREE.MathUtils.clamp(state.openness,0,1):0);
  const round=preview?(Math.sin(time*3)+1)/2:THREE.MathUtils.clamp(state.round||0,0,1);
  const wide=preview?(Math.cos(time*4)+1)/2:THREE.MathUtils.clamp(state.wide||0,0,1);
  if(now-lastUi>50){
    const status=preview?'无声动作预览 · 非实际对话':connected?stateNames[state.state]||state.state:'语音连接已断开';
    if($('status').textContent!==status)$('status').textContent=status;
    bars.forEach((bar,i)=>{bar.style.transform=`scaleY(${0.11+0.89*(speaking?open:state.mic||0)*(0.35+0.65*Math.sin(i*1.7+time*8)**2)})`;});
    lastUi=now;
  }
  if(vrm){
    const weights={aa:open*(1-round)*0.7,ih:open*wide*0.18,ee:open*wide*0.12,oh:open*round*0.65,ou:open*round*0.35};
    // Audio energy is not a phoneme recognizer: these are smooth approximate vowels.
    const factor=1-Math.exp(-dt*(open>smooth.aa?24:17));
    for(const name in smooth){smooth[name]+=(weights[name]-smooth[name])*factor;vrm.expressionManager.setValue(name,smooth[name]);}
    if(time>blinkAt){blinkStart=time;blinkAt=time+2.6+Math.random()*3.5;}
    const phase=(time-blinkStart)/0.17;
    vrm.expressionManager.setValue('blink',phase<1?Math.sin(phase*Math.PI):0);
    vrm.expressionManager.setValue('relaxed',0.10);
    vrm.expressionManager.setValue('happy',speaking?0.12:0.06);
    const talk=speaking?1:0.25;
    bone('head').rotation.set(Math.sin(time*1.1)*0.024+talk*Math.sin(time*3.2)*0.012,Math.sin(time*0.54)*0.055,Math.sin(time*0.72)*0.025);
    bone('neck').rotation.x=Math.sin(time*1.1-0.4)*0.012;
    bone('chest').rotation.set(Math.sin(time*1.7)*0.008,Math.sin(time*0.5)*0.018,Math.sin(time*0.6)*0.009);
    bone('spine').rotation.z=Math.sin(time*0.6)*0.008;
    bone('leftUpperArm').rotation.z=-1.42-Math.sin(time*1.7)*0.009;
    bone('rightUpperArm').rotation.z=1.42+Math.sin(time*1.7)*0.009;
    if(time>lookNext){gaze.position.set((Math.random()-.5)*0.18,1.35+(Math.random()-.5)*0.10,3);lookNext=time+2+Math.random()*3;}
    const beforeUpdate=performance.now();
    vrm.update(dt);
    window.avatarDiagnostics={updateMs:performance.now()-beforeUpdate,pixelRatio:renderer.getPixelRatio(),mouth:{...smooth},sourceState:state.state,preview};
  }
  const beforeRender=performance.now();
  renderer.render(scene,camera);frames++;frameTimes.push(rawDelta*1000);
  if(window.avatarDiagnostics)window.avatarDiagnostics.renderMs=performance.now()-beforeRender;
  if(now-sampleStart>2000){
    const ordered=frameTimes.slice().sort((a,b)=>a-b);
    const fps=frames*1000/(now-sampleStart),p95=ordered[Math.floor(ordered.length*.95)];
    window.avatarMetrics={fps,p95ms:p95,triangles:renderer.info.render.triangles,calls:renderer.info.render.calls};
    $('metrics').textContent=`${fps.toFixed(1)} FPS · P95 ${p95.toFixed(0)} ms · ${renderer.info.render.calls} draws`;
    frames=0;frameTimes=[];sampleStart=now;
  }
}requestAnimationFrame(animate);