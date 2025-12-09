#include "PathResolver.h"
#include <SDL.h>
#include <fstream>
#include <algorithm>
#include <vector>

std::string PathResolver::ResolvePath(const std::string& relativePath)
{
    // Normalizar o caminho relativo primeiro
    std::string normalized = NormalizePath(relativePath);
    
    // Lista de caminhos possíveis para tentar
    std::vector<std::string> possiblePaths;
    
    // 1. Tentar caminho relativo direto (para compatibilidade com execução do diretório de build)
    possiblePaths.push_back(normalized);
    
    // 2. Tentar com "../" (executável em subdiretório como cmake-build-debug)
    if (normalized.find("../") != 0) {
        possiblePaths.push_back("../" + normalized);
    }
    
    // 3. Tentar com "./" (executável no diretório raiz)
    possiblePaths.push_back("./" + normalized);
    
    // 4. Usar SDL_GetBasePath() para obter o diretório do executável
    char* basePath = SDL_GetBasePath();
    if (basePath) {
        std::string base(basePath);
        SDL_free(basePath);
        
        // Normalizar separadores do basePath
        base = NormalizePath(base);
        
        // Remover barra final se existir
        if (!base.empty() && (base.back() == '/' || base.back() == '\\')) {
            base.pop_back();
        }
        
        // Tentar no diretório do executável
        possiblePaths.push_back(base + "/" + normalized);
        
        // Tentar no diretório pai do executável (útil quando executável está em build/)
        size_t lastSlash = base.find_last_of('/');
        if (lastSlash != std::string::npos) {
            std::string parent = base.substr(0, lastSlash);
            possiblePaths.push_back(parent + "/" + normalized);
            
            // Tentar também no diretório raiz do projeto (2 níveis acima)
            size_t secondLastSlash = parent.find_last_of('/');
            if (secondLastSlash != std::string::npos) {
                std::string projectRoot = parent.substr(0, secondLastSlash);
                possiblePaths.push_back(projectRoot + "/" + normalized);
            }
        }
    }
    
    // Tentar cada caminho e retornar o primeiro que existir
    for (const auto& path : possiblePaths) {
        if (FileExists(path)) {
            return path;
        }
    }
    
    // Se nenhum caminho for encontrado, retornar o normalizado original
    // (o código que chama deve tratar o erro se o arquivo não existir)
    return normalized;
}

std::string PathResolver::NormalizePath(const std::string& path)
{
    std::string normalized = path;
    
    // Substituir todas as barras invertidas por barras normais
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    
    // Remover barras duplicadas (exceto no início para URLs)
    std::string result;
    result.reserve(normalized.length());
    bool lastWasSlash = false;
    for (size_t i = 0; i < normalized.length(); ++i) {
        if (normalized[i] == '/') {
            if (!lastWasSlash) {
                result += '/';
                lastWasSlash = true;
            }
        } else {
            result += normalized[i];
            lastWasSlash = false;
        }
    }
    
    return result;
}

bool PathResolver::FileExists(const std::string& path)
{
    std::ifstream file(path);
    return file.good();
}

