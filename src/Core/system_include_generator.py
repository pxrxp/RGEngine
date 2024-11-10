import os

AUTO_GENERATED_COMMENT = (
    '/*\n'
    ' * This file is auto-generated. Do not modify it manually.\n'
    ' * Any changes made will be overwritten the next time the script is run.\n'
    ' * <Core/ServiceLocator.h> must be included before calling the macro.\n'
    ' */\n\n'
)

script_path = os.path.abspath(__file__)
script_dir = os.path.dirname(script_path)
parent_dir = os.path.dirname(script_dir)
systems_dir = os.path.join(parent_dir, 'Systems')

out_h_file = os.path.join(script_dir, 'SystemInitializer.h')

def writeIncludeGuards(lines):
    lines.append(f'#ifndef SYSTEM_INITIALIZER_H')
    lines.append(f'#define SYSTEM_INITIALIZER_H')
    lines.append('')

def writeIncludes(lines, system):
    lines.append(f'#include <Systems/{system}.h>')

def writeMacro(lines):
    lines.append('#define INITIALIZE_SYSTEMS()\\')

def writeInstance(lines, system):
    lines.append(f'    static {system} {system}_instance;\\')

def writeGetSystemManager(lines):
    lines.append('    ServiceLocator::get_system_manager()\\')

def addInstance(lines, system):
    lines.append(f'        .add({system}_instance)\\')

def writeEnd(lines):
    lines.append('        .register_events();')
    lines.append('')
    lines.append('#endif // SYSTEM_INITIALIZER_H')

def alignBackslashes(lines):
    max_length = max(len(line) for line in lines if line.endswith('\\'))

    for i, line in enumerate(lines):
        if not line.endswith('\\'):
            continue

        beforeBackslash = line[:-1]
        whitespace = ' ' * (max_length-len(line)+1)
        lines[i] = f"{beforeBackslash}{whitespace}\\"



lines = []
lines.append(AUTO_GENERATED_COMMENT)
writeIncludeGuards(lines)
system_files = [os.path.splitext(f)[0] for f in os.listdir(systems_dir) if f.endswith('.h')]

for file in system_files:
    writeIncludes(lines, file)
lines.append('')

writeMacro(lines)

for file in system_files:
    writeInstance(lines, file)
lines.append('\\')

writeGetSystemManager(lines)

for file in system_files:
    addInstance(lines, file)

writeEnd(lines)
alignBackslashes(lines)

with open(out_h_file, 'w') as out_h:
    out_h.write('\n'.join(lines))
