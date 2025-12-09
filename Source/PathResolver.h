#pragma once
#include <string>

// Classe utilitária para resolver caminhos de recursos de forma portável
// Funciona em Windows, Linux e macOS
class PathResolver
{
public:
    // Resolve o caminho para um arquivo de recurso
    // Tenta múltiplos locais possíveis baseado no diretório do executável
    // Retorna o primeiro caminho válido encontrado, ou o caminho original se nenhum for encontrado
    static std::string ResolvePath(const std::string& relativePath);
    
    // Normaliza separadores de caminho para usar '/' (compatível com todos os sistemas)
    static std::string NormalizePath(const std::string& path);
    
    // Verifica se um arquivo existe
    static bool FileExists(const std::string& path);
};

