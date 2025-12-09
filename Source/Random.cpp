#include "Random.h"

// Inicializa o gerador de números aleatórios com uma semente aleatória
void Random::Init()
{
    std::random_device rd;
    Random::Seed(rd());
}

// Define a semente do gerador de números aleatórios
void Random::Seed(unsigned int seed)
{
    sGenerator.seed(seed);
}

// Retorna um float aleatório entre 0.0 e 1.0
float Random::GetFloat()
{
    return GetFloatRange(0.0f, 1.0f);
}

// Retorna um float aleatório no intervalo [min, max)
float Random::GetFloatRange(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(sGenerator);
}

// Retorna um int aleatório no intervalo [min, max]
int Random::GetIntRange(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(sGenerator);
}

// Retorna um Vector2 aleatório no intervalo especificado
Vector2 Random::GetVector(const Vector2& min, const Vector2& max)
{
    Vector2 r = Vector2(GetFloat(), GetFloat());
    return min + (max - min) * r;
}

// Retorna um Vector3 aleatório no intervalo especificado
Vector3 Random::GetVector(const Vector3& min, const Vector3& max)
{
    Vector3 r = Vector3(GetFloat(), GetFloat(), GetFloat());
    return min + (max - min) * r;
}

std::mt19937 Random::sGenerator;
