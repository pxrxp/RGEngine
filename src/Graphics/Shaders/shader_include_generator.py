import os

AUTO_GENERATED_COMMENT = (
    '/*\n'
    ' * This file is auto-generated. Do not modify it manually.\n'
    ' * Any changes made will be overwritten the next time the script is run.\n'
    ' */\n\n'
)

SHADER_TYPE_MAP = {
    'vert': 'GL_VERTEX_SHADER',
    'frag': 'GL_FRAGMENT_SHADER',
    'tc': 'GL_TESS_CONTROL_SHADER',
    'te': 'GL_TESS_EVALUATION_SHADER',
    'geo': 'GL_GEOMETRY_SHADER',
    'comp': 'GL_COMPUTE_SHADER'
}

script_path = os.path.abspath(__file__)
script_dir = os.path.dirname(script_path)
parent_dir = os.path.dirname(script_dir)
grandparent_dir = os.path.dirname(parent_dir)
shaders_dir = os.path.join(grandparent_dir, 'shaders')

out_h_file = os.path.join(script_dir, 'ShaderIncludes.h')
out_cpp_file = os.path.join(script_dir, 'ShaderIncludes.cpp')

def writeComments(out_cpp, out_h):
    out_h.write(AUTO_GENERATED_COMMENT)
    out_cpp.write(AUTO_GENERATED_COMMENT)

def writePreProcessing(out_cpp):
    out_cpp.write('#include <Graphics/Shaders/ShaderIncludes.h>\n')
    out_cpp.write('#include <Graphics/Shaders/ShaderManager.h>\n')
    out_cpp.write('#include <glad/gl.h>\n\n')

def isShaderAliasOfAnother(filename):
    return filename.startswith('_')

def removePrefixUnderscore(cstr_program):
    return cstr_program[1:]

def writeIncludesForShaderFiles(out_h, cstr_shaderType_glsl, filename):
    out_h.write(f'inline constexpr const char * {cstr_shaderType_glsl} =\n')
    out_h.write(f'\t#include <shaders/{filename}>\n')
    out_h.write(';\n\n')

def writeIncludeGuardsPart1(out_h):
    out_h.write('\n#ifndef SHADER_INCLUDES_H\n')
    out_h.write('#define SHADER_INCLUDES_H\n\n')

def writeIncludeGuardsPart2(out_h):
    out_h.write('#endif // SHADER_INCLUDES_H')

def writeShaderCompiles(out_cpp, shader_type, cstr_shaderType_shader):
    GL_TYPE_SHADER = SHADER_TYPE_MAP[shader_type]
    out_cpp.write(f'\tGLuint {cstr_shaderType_shader} = glCreateShader({GL_TYPE_SHADER});\n')
    out_cpp.write(f'\tglShaderSource({cstr_shaderType_shader}, 1, &{cstr_shaderType_glsl}, nullptr);\n')
    out_cpp.write(f'\tglCompileShader({cstr_shaderType_shader});\n\n')

def writeProgram(out_cpp, program_name, shaders_list):
    out_cpp.write(f'\tGLuint {program_name} = glCreateProgram();\n')

    for shader_name in shaders_list:
        out_cpp.write(f'\tglAttachShader({program_name}, {shader_name});\n')

    out_cpp.write(f'\tglLinkProgram({program_name});\n')

    out_cpp.write(f'\tdatabase["{cstr}"] = {program_name};\n\n')

def writeDeletes(out_cpp, shader_name):
    out_cpp.write(f'\tglDeleteShader({shader_name});\n')

def startFunction(out_cpp):
    out_cpp.write('ShaderManager::ShaderManager() {\n')

def endFunction(out_cpp):
    out_cpp.write('}')

with open(out_h_file, 'w') as out_h, \
    open(out_cpp_file, 'w') as out_cpp:

    writeComments(out_cpp, out_h)
    writePreProcessing(out_cpp)
    startFunction(out_cpp)

    shader_files = [f for f in os.listdir(shaders_dir) if f.endswith('.glsl')]
    existing_shaders = set()
    program_shaders = {}
    
    writeIncludeGuardsPart1(out_h)
    for filename in shader_files:
        program, shader_type, _ = filename.split('.')
        cstr = program.replace('.', '_')
        cstr_program = f"{cstr}_program"

        if isShaderAliasOfAnother(filename):
            with open(os.path.join(shaders_dir, filename)) as glsl_file:
                aliasOf = glsl_file.readline().strip()
                filename = aliasOf
            cstr_program = removePrefixUnderscore(cstr_program)

        cstr_shaderType_glsl = filename.replace('.', '_')
        cstr_shaderType_shader = cstr_shaderType_glsl.replace('glsl', 'shader')

        if cstr_program not in program_shaders:
            program_shaders[cstr_program] = []

        program_shaders[cstr_program].append(cstr_shaderType_shader)

        if cstr_shaderType_shader in existing_shaders:
            continue

        writeIncludesForShaderFiles(out_h, cstr_shaderType_glsl, filename)

        writeShaderCompiles(out_cpp, shader_type, cstr_shaderType_shader)

        existing_shaders.add(cstr_shaderType_shader)

    writeIncludeGuardsPart2(out_h)

    out_cpp.write(f'\n')
    for program_name, shaders_list in program_shaders.items():
        writeProgram(out_cpp, program_name, shaders_list)

    out_cpp.write(f'\n')
    for shader_name in existing_shaders:
        writeDeletes(out_cpp, shader_name)

    endFunction(out_cpp)
