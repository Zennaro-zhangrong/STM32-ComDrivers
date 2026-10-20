"""逐demo运行回归测试；产物仅写入各demo的Tests/build，不创建顶层辅助目录。"""
from pathlib import Path
import argparse
import json
import os
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--all', action='store_true', help='验证仓库内全部demo；默认FOC')
    parser.add_argument('--host-compiler', type=Path, help='可选的本机C编译器，如gcc、clang或tcc')
    parser.add_argument('--source-root', type=Path, help='仅读取既有源项目HAL/LVGL头文件进行可选适配编译')
    args = parser.parse_args()
    demos = sorted(ROOT.glob('*_demo')) if args.all else [ROOT/'FOC_demo']
    arm = shutil.which('arm-none-eabi-gcc')
    if not arm:
        parser.error('需要arm-none-eabi-gcc进行严格目标编译')
    host = args.host_compiler
    if host is None:
        local = ROOT/'FOC_demo/Tests/build/compiler/tcc/tcc.exe'
        if local.is_file(): host = local
    report = {'demos':[], 'checks':[]}
    output = ROOT/'FOC_demo/Tests/build'
    output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi=hard']
    all_objects=[]
    all_headers=[]
    hal=[]; lvgl=[]
    if args.source_root:
        car=args.source_root.resolve()/'CarMainControlBoard'
        hal=['-DSTM32F407xx','-DUSE_HAL_DRIVER']
        for folder in ['Core/Inc','Drivers/STM32F4xx_HAL_Driver/Inc','Drivers/CMSIS/Device/ST/STM32F4xx/Include','Drivers/CMSIS/Include']:
            hal+=['-I',str(car/folder)]
        display=args.source_root.resolve()/'LVGL-_stm32f407vet6'
        lvgl=['-I',str(display/'lvgl'),'-I',str(display)]
    def run(command, label):
        result=subprocess.run([str(x) for x in command],cwd=ROOT,capture_output=True,text=True,timeout=60)
        report['checks'].append({'name':label,'returncode':result.returncode,'output':result.stdout+result.stderr})
        if result.returncode:
            raise RuntimeError(label+'\n'+result.stdout+result.stderr)
    try:
        for demo in demos:
            build=demo/'Tests/build'
            build.mkdir(parents=True,exist_ok=True)
            sources=[]; objects=[]
            for layer in ['Mapping','Middleware','Service']:
                for path in sorted((demo/layer).rglob('*.[ch]')):
                    optional='STM32F4' in path.parts or 'Lvgl8' in path.parts
                    if optional and not args.source_root: continue
                    text=path.read_text(encoding='utf-8')
                    for name in re.findall(r'#\s*include\s*"([^"]+)"',text):
                        if name in ['stm32f4xx_hal.h','lvgl.h']: continue
                        target=(path.parent/name).resolve()
                        assert target.is_file() and target.is_relative_to(demo.resolve()),(path,name)
                        if layer=='Middleware': assert target.is_relative_to((demo/layer).resolve()),path
                        if layer=='Mapping': assert target.is_relative_to((demo/layer).resolve()),path
                    stem='_'.join(path.relative_to(demo).parts).replace('.','_')
                    object_path=build/(stem+'.o')
                    source=path
                    if path.suffix=='.h':
                        source=build/(stem+'.c')
                        source.write_text('#include "'+path.as_posix()+'"\n',encoding='utf-8')
                        all_headers.append(path)
                    extra=hal if 'STM32F4' in path.parts else lvgl if 'Lvgl8' in path.parts else []
                    run([arm,*flags,*extra,'-c',source,'-o',object_path],demo.name+'/'+str(path.relative_to(demo)))
                    if path.suffix=='.c':
                        all_objects.append(object_path)
                        if not optional: objects.append(object_path); sources.append(path)
            test=demo/'Tests/test_regression.c'
            run([arm,*flags,'-c',test,'-o',build/'regression.o'],demo.name+' test strict ARM compile')
            run([arm,*flags,'--specs=nosys.specs',*objects,build/'regression.o','-lm','-o',build/'regression.elf'],demo.name+' ARM link')
            if host:
                executable=build/('regression.exe' if os.name=='nt' else 'regression')
                math_library=[] if os.name=='nt' and host.name.lower()=='tcc.exe' else ['-lm']
                compatibility=[ROOT/'FOC_demo/Tests/tcc_math.c'] if os.name=='nt' and host.name.lower()=='tcc.exe' else []
                host_flags=['-include',ROOT/'FOC_demo/Tests/tcc_compat.h'] if compatibility else []
                run([host,'-std=c11','-Wall',*host_flags,test,*sources,*compatibility,*math_library,'-o',executable],demo.name+' host compile')
                run([executable],demo.name+' host execution')
            report['demos'].append(demo.name)
            print('PASS:',demo.name,'ARM compile/link'+(' + host execution' if host else ' (NOT executed)'),flush=True)
        combined=output/'all_headers.c'
        combined.write_text('\n'.join('#include "'+p.as_posix()+'"' for p in all_headers)+'\n',encoding='utf-8')
        run([arm,*flags,*hal,*lvgl,'-c',combined,'-o',output/'all_headers.o'],'combined public headers')
        run([arm,*flags,'-nostdlib','-r',*all_objects,'-o',output/'combined.o'],'combined symbols')
        if args.all and (ROOT/'LOG_demo').is_dir():
            pair=Path(tempfile.mkdtemp(prefix='foc-log-',dir=output))
            for name in ['FOC_demo','LOG_demo']:
                for layer in ['Mapping','Middleware','Service']:
                    shutil.copytree(ROOT/name/layer,pair/name/layer)
            pair_test=pair/'test_foc_log.c'
            shutil.copyfile(ROOT/'FOC_demo/Tests/test_foc_log.c',pair_test)
            pair_sources=[p for name in ['FOC_demo','LOG_demo'] for layer in ['Middleware','Service'] for p in (pair/name/layer).rglob('*.c')]
            run([arm,*flags,'-I',pair,pair_test,*pair_sources,'--specs=nosys.specs','-lm','-o',pair/'combined.elf'],'isolated FOC + LOG ARM link')
            if host:
                executable=pair/('combined.exe' if os.name=='nt' else 'combined')
                run([host,'-std=c11','-Wall',*host_flags,'-I',pair,pair_test,*pair_sources,*compatibility,*math_library,'-o',executable],'isolated FOC + LOG host compile')
                run([executable],'isolated FOC + LOG host execution')
    finally:
        (output/'regression_report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    print('PASS: all requested checks; no hardware tests performed')

if __name__=='__main__': main()
