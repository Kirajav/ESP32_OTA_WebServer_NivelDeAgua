#!/usr/bin/env python3
"""
Script de minificación automática para archivos web
Se ejecuta como pre-build hook de PlatformIO
"""

import os
import re
from pathlib import Path

# Determinar el directorio del proyecto
try:
    # Cuando se ejecuta desde PlatformIO
    Import("env")
    PROJECT_DIR = Path(env.get("PROJECT_DIR"))
except:
    # Cuando se ejecuta manualmente
    PROJECT_DIR = Path(__file__).parent.parent

DATA_WEB_DIR = PROJECT_DIR / "data" / "web"

def minify_js(content):
    """
    Minificación AGRESIVA de JavaScript
    Elimina: comentarios, espacios innecesarios, líneas vacías, múltiples espacios
    """
    # Eliminar comentarios de bloque /* ... */
    content = re.sub(r'/\*[\s\S]*?\*/', '', content)
    
    # Eliminar comentarios de línea //
    content = re.sub(r'//[^\n]*', '', content)
    
    # Eliminar líneas vacías múltiples
    content = re.sub(r'\n\s*\n+', '\n', content)
    
    # Unir todo en una sola línea primero
    lines = content.split('\n')
    content = ' '.join(line.strip() for line in lines if line.strip())
    
    # Eliminar espacios alrededor de operadores y símbolos
    content = re.sub(r'\s*([{}\[\]();:,=+\-*/<>!&|?])\s*', r'\1', content)
    
    # Mantener un solo espacio después de palabras clave JavaScript
    keywords = ['const', 'let', 'var', 'function', 'return', 'if', 'else', 
                'for', 'while', 'case', 'break', 'continue', 'new', 'typeof',
                'async', 'await', 'catch', 'try', 'throw', 'class', 'extends']
    for keyword in keywords:
        content = re.sub(rf'\b{keyword}(?=[a-zA-Z_$])', rf'{keyword} ', content)
    
    # Eliminar espacios múltiples
    content = re.sub(r' +', ' ', content)
    
    return content.strip()

def minify_css(content):
    """
    Minificación AGRESIVA de CSS
    """
    # Eliminar comentarios
    content = re.sub(r'/\*[\s\S]*?\*/', '', content)
    
    # Unir todo en una sola línea
    lines = content.split('\n')
    content = ' '.join(line.strip() for line in lines if line.strip())
    
    # Eliminar espacios alrededor de símbolos
    content = re.sub(r'\s*([{};:,>+~])\s*', r'\1', content)
    
    # Eliminar espacios múltiples
    content = re.sub(r' +', ' ', content)
    
    # Eliminar espacios antes de !
    content = re.sub(r' !', '!', content)
    
    return content.strip()

def minify_html(content):
    """
    Minificación AGRESIVA de HTML
    Preserva scripts y estilos inline
    """
    # Eliminar comentarios HTML (pero no los condicionales de IE)
    content = re.sub(r'<!--(?!\[if)[\s\S]*?-->', '', content)
    
    # Eliminar espacios entre tags
    content = re.sub(r'>\s+<', '><', content)
    
    # Eliminar líneas vacías y espacios múltiples
    content = re.sub(r'\n\s*\n+', '', content)
    content = re.sub(r'\s+', ' ', content)
    
    # Eliminar espacios al inicio/final
    return content.strip()

def process_file(src_path):
    """
    Procesa un archivo desde /src y genera versión minificada en parent
    Estructura: data/web/dashboard/src/dashboard.js → data/web/dashboard/dashboard.min.js
    """
    try:
        with open(src_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_size = len(content)
        ext = src_path.suffix.lower()
        
        # Solo minificar JS, CSS y HTML
        if ext not in ['.js', '.css', '.html']:
            return original_size, original_size, False
        
        # Aplicar minificación según tipo
        if ext == '.js':
            minified_content = minify_js(content)
        elif ext == '.css':
            minified_content = minify_css(content)
        elif ext == '.html':
            minified_content = minify_html(content)
        else:
            return original_size, original_size, False
        
        # Guardar en directorio padre (fuera de /src)
        # data/web/dashboard/src/dashboard.js → data/web/dashboard/dashboard.min.js
        parent_dir = src_path.parent.parent
        min_filename = f"{src_path.stem}.min{src_path.suffix}"
        min_path = parent_dir / min_filename
        
        # Guardar archivo minificado en raíz del módulo web
        with open(min_path, 'w', encoding='utf-8') as f:
            f.write(minified_content)
        
        final_size = len(minified_content)
        return original_size, final_size, True, str(min_path.relative_to(DATA_WEB_DIR))
        
    except Exception as e:
        print(f"❌ Error procesando {src_path.name}: {e}")
        return 0, 0, False, ""

def minify_web_files():
    """
    Minifica archivos desde carpetas /src hacia directorio padre
    Estructura: data/web/{module}/src/*.{js,css,html} → data/web/{module}/*.min.{ext}
    """
    print("\n🔧 Iniciando minificación de archivos web...")
    print("📂 Buscando archivos en carpetas /src...\n")
    
    if not DATA_WEB_DIR.exists():
        print(f"⚠️  Directorio {DATA_WEB_DIR} no encontrado")
        return
    
    total_original = 0
    total_final = 0
    processed_files = 0
    
    # Buscar carpetas /src dentro de data/web
    src_dirs = list(DATA_WEB_DIR.rglob("src"))
    
    if not src_dirs:
        print("⚠️  No se encontraron carpetas /src en data/web/")
        return
    
    for src_dir in src_dirs:
        module_name = src_dir.parent.name
        print(f"📦 Procesando módulo: {module_name}")
        
        # Procesar archivos JS, CSS, HTML en carpeta /src
        for file_path in src_dir.glob("*"):
            if file_path.suffix.lower() in ['.js', '.css', '.html']:
                orig_size, final_size, processed, output_path = process_file(file_path)
                
                if processed:
                    total_original += orig_size
                    total_final += final_size
                    processed_files += 1
                    reduction = ((orig_size - final_size) / orig_size * 100) if orig_size > 0 else 0
                    print(f"  ✓ {file_path.name} → {output_path}")
                    print(f"    {orig_size:,} → {final_size:,} bytes ({reduction:.1f}% reducción)")
        
        print()  # Línea en blanco entre módulos
    
    if processed_files > 0:
        total_reduction = ((total_original - total_final) / total_original * 100) if total_original > 0 else 0
        print(f"✅ Minificación completada:")
        print(f"   📊 Archivos procesados: {processed_files}")
        print(f"   📏 Tamaño original: {total_original:,} bytes ({total_original/1024:.1f} KB)")
        print(f"   📉 Tamaño final: {total_final:,} bytes ({total_final/1024:.1f} KB)")
        print(f"   💾 Reducción total: {total_reduction:.1f}% (~{(total_original-total_final)/1024:.1f} KB ahorrados)")
        print(f"\n   💡 Archivos fuente: data/web/*/src/")
        print(f"   💡 Archivos minificados: data/web/*/*.min.*")
        print(f"   ✅ Listos para versionar en Git\n")
    else:
        print(f"⚠️  No se encontraron archivos para minificar en carpetas /src\n")

# Ejecutar minificación cuando se importa desde PlatformIO
minify_web_files()
