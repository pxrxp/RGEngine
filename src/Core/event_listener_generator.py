import os

AUTO_GENERATED_COMMENT = (
    '/*\n'
    ' * This file is auto-generated. Do not modify it manually.\n'
    ' * Any changes made will be overwritten the next time the script is run.\n'
    ' */\n\n'
)

script_path = os.path.abspath(__file__)
script_dir = os.path.dirname(script_path)
parent_dir = os.path.dirname(script_dir)
events_dir = os.path.join(parent_dir, 'Events')

out_h_file = os.path.join(script_dir, 'EventListener.h')

def writeIncludeGuards(out_h):
    out_h.write(f'#ifndef EVENT_LISTENER_H\n')
    out_h.write(f'#define EVENT_LISTENER_H\n\n')

def writeForwardDeclarations(out_h, event):
    out_h.write(f'class {event};\n')

def writeClass(out_h):
    out_h.write(f'class EventListener {{\n')
    out_h.write(f'public:\n')

def writeListenFunction(out_h, event):
    out_h.write(f'\tvirtual void listen(const {event}& event) {{}}\n')

def writeEnd(out_h):
    out_h.write(f'}};\n\n')
    out_h.write(f'#endif // EVENT_LISTENER_H')

with open(out_h_file, 'w') as out_h:
    out_h.write(AUTO_GENERATED_COMMENT)
    writeIncludeGuards(out_h)

    event_files = [os.path.splitext(f)[0] for f in os.listdir(events_dir) if f.endswith('.h')]

    for file in event_files:
        writeForwardDeclarations(out_h, file)
    out_h.write('\n')

    writeClass(out_h)

    for file in event_files:
        writeListenFunction(out_h, file)

    writeEnd(out_h)
