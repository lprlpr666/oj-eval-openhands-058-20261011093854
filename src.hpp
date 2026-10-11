#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <cstring>

class BasicException {
protected:
    const char *message;

public:
    explicit BasicException(const char *_message) : message(_message) {}

    virtual const char *what() const {
        return message;
    }
};

class ArgumentException: public BasicException {
public:
    explicit ArgumentException(const char *_message) : BasicException(_message) {}
};

class IteratorException: public BasicException {
public:
    explicit IteratorException(const char *_message) : BasicException(_message) {}
};

struct Pokemon {
    char name[12];
    int id;
    std::vector<std::string> types;
    
    Pokemon() : id(0) {
        memset(name, 0, sizeof(name));
    }
    
    Pokemon(const char *_name, int _id, const std::vector<std::string>& _types) : id(_id), types(_types) {
        strncpy(name, _name, 11);
        name[11] = '\0';
    }
};

class Pokedex {
private:
    std::map<int, Pokemon> pokemonMap;
    std::string fileName;
    
    // Type effectiveness mapping
    std::map<std::string, std::map<std::string, float>> typeChart;
    
    void initializeTypeChart() {
        // Initialize all to 1.0 (neutral)
        std::vector<std::string> types = {"normal", "fire", "water", "grass", "electric", "ice", "fighting"};
        for (const auto& attacker : types) {
            for (const auto& defender : types) {
                typeChart[attacker][defender] = 1.0f;
            }
        }
        
        // Set up effectiveness based on the problem description
        // Normal
        typeChart["normal"]["fighting"] = 0.5f;
        
        // Fire
        typeChart["fire"]["grass"] = 2.0f;
        typeChart["fire"]["ice"] = 2.0f;
        typeChart["fire"]["fire"] = 0.5f;
        typeChart["fire"]["water"] = 0.5f;
        
        // Water
        typeChart["water"]["fire"] = 2.0f;
        typeChart["water"]["ground"] = 2.0f;
        typeChart["water"]["water"] = 0.5f;
        typeChart["water"]["grass"] = 0.5f;
        
        // Grass
        typeChart["grass"]["water"] = 2.0f;
        typeChart["grass"]["ground"] = 2.0f;
        typeChart["grass"]["fire"] = 0.5f;
        typeChart["grass"]["grass"] = 0.5f;
        typeChart["grass"]["electric"] = 0.5f;
        
        // Electric
        typeChart["electric"]["water"] = 2.0f;
        typeChart["electric"]["flying"] = 2.0f;
        typeChart["electric"]["electric"] = 0.5f;
        typeChart["electric"]["grass"] = 0.5f;
        typeChart["electric"]["ground"] = 0.0f;
        
        // Ice
        typeChart["ice"]["grass"] = 2.0f;
        typeChart["ice"]["ground"] = 2.0f;
        typeChart["ice"]["flying"] = 2.0f;
        typeChart["ice"]["fire"] = 0.5f;
        typeChart["ice"]["water"] = 0.5f;
        typeChart["ice"]["ice"] = 0.5f;
        
        // Fighting
        typeChart["fighting"]["normal"] = 2.0f;
        typeChart["fighting"]["ice"] = 2.0f;
    }
    
    bool isValidName(const char *name) {
        if (!name || strlen(name) == 0 || strlen(name) > 10) return false;
        for (size_t i = 0; i < strlen(name); i++) {
            if (!isalpha(name[i])) return false;
        }
        return true;
    }
    
    bool isValidType(const std::string &type) const {
        std::vector<std::string> validTypes = {"normal", "fire", "water", "grass", "electric", "ice", "fighting"};
        return std::find(validTypes.begin(), validTypes.end(), type) != validTypes.end();
    }
    
    std::vector<std::string> parseTypes(const char *types) const {
        std::vector<std::string> result;
        std::string str(types);
        std::stringstream ss(str);
        std::string token;
        
        while (std::getline(ss, token, '#')) {
            if (!token.empty()) {
                result.push_back(token);
            }
        }
        return result;
    }
    
    void saveToFile() {
        std::ofstream file(fileName);
        if (!file.is_open()) return;
        
        for (const auto& pair : pokemonMap) {
            const Pokemon& pokemon = pair.second;
            file << pokemon.name << " " << pokemon.id;
            for (size_t i = 0; i < pokemon.types.size(); i++) {
                if (i == 0) file << " ";
                else file << "#";
                file << pokemon.types[i];
            }
            file << std::endl;
        }
        
        file.close();
    }
    
    void loadFromFile() {
        std::ifstream file(fileName);
        if (!file.is_open()) return;
        
        pokemonMap.clear();
        
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string name, typesStr;
            int id;
            
            if (!(ss >> name >> id)) continue;
            
            std::string remaining;
            std::getline(ss, remaining);
            if (!remaining.empty() && remaining[0] == ' ') {
                remaining = remaining.substr(1);
            }
            
            std::vector<std::string> types = parseTypes(remaining.c_str());
            
            Pokemon pokemon(name.c_str(), id, types);
            pokemonMap[id] = pokemon;
        }
        
        file.close();
    }

public:
    explicit Pokedex(const char *_fileName) : fileName(_fileName) {
        initializeTypeChart();
        loadFromFile();
    }

    ~Pokedex() {
        saveToFile();
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        // Validate name
        if (!isValidName(name)) {
            throw ArgumentException(("Argument Error: PM Name Invalid (" + std::string(name) + ")").c_str());
        }
        
        // Check if name already exists
        for (const auto& pair : pokemonMap) {
            if (strcmp(pair.second.name, name) == 0) {
                throw ArgumentException(("Argument Error: PM Name Duplicate (" + std::string(name) + ")").c_str());
            }
        }
        
        // Check if ID already exists
        if (pokemonMap.find(id) != pokemonMap.end()) {
            throw ArgumentException(("Argument Error: PM ID Duplicate (" + std::to_string(id) + ")").c_str());
        }
        
        // Parse and validate types
        std::vector<std::string> typeList = parseTypes(types);
        if (typeList.empty() || typeList.size() > 7) {
            throw ArgumentException("Argument Error: PM Type Invalid ()");
        }
        
        for (const auto& type : typeList) {
            if (!isValidType(type)) {
                throw ArgumentException(("Argument Error: PM Type Invalid (" + type + ")").c_str());
            }
        }
        
        // Check for duplicate types
        std::set<std::string> uniqueTypes(typeList.begin(), typeList.end());
        if (uniqueTypes.size() != typeList.size()) {
            throw ArgumentException("Argument Error: PM Type Duplicate");
        }
        
        Pokemon pokemon(name, id, typeList);
        pokemonMap[id] = pokemon;
        return true;
    }

    bool pokeDel(int id) {
        if (pokemonMap.find(id) == pokemonMap.end()) {
            return false;
        }
        pokemonMap.erase(id);
        return true;
    }

    std::string pokeFind(int id) const {
        auto it = pokemonMap.find(id);
        if (it == pokemonMap.end()) {
            return "None";
        }
        return std::string(it->second.name);
    }

    std::string typeFind(const char *types) const {
        std::vector<std::string> searchTypes = parseTypes(types);
        
        // Validate search types
        for (const auto& type : searchTypes) {
            if (!isValidType(type)) {
                throw ArgumentException(("Argument Error: PM Type Invalid (" + type + ")").c_str());
            }
        }
        
        std::vector<Pokemon> results;
        
        for (const auto& pair : pokemonMap) {
            const Pokemon& pokemon = pair.second;
            bool match = true;
            
            for (const auto& searchType : searchTypes) {
                bool found = false;
                for (const auto& pokemonType : pokemon.types) {
                    if (pokemonType == searchType) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    match = false;
                    break;
                }
            }
            
            if (match) {
                results.push_back(pokemon);
            }
        }
        
        if (results.empty()) {
            return "None";
        }
        
        // Sort by ID
        std::sort(results.begin(), results.end(), [](const Pokemon& a, const Pokemon& b) {
            return a.id < b.id;
        });
        
        std::string result = std::to_string(results.size());
        for (const auto& pokemon : results) {
            result += "\n" + std::string(pokemon.name);
        }
        
        return result;
    }

    float attack(const char *type, int id) const {
        if (pokemonMap.find(id) == pokemonMap.end()) {
            return -1.0f;
        }
        
        const Pokemon& defender = pokemonMap.at(id);
        std::string attackType(type);
        
        float totalDamage = 1.0f;
        
        for (const auto& defenseType : defender.types) {
            if (typeChart.find(attackType) != typeChart.end() && 
                typeChart.at(attackType).find(defenseType) != typeChart.at(attackType).end()) {
                totalDamage *= typeChart.at(attackType).at(defenseType);
            }
        }
        
        return totalDamage;
    }

    int catchTry() const {
        if (pokemonMap.empty()) {
            return 0;
        }
        
        // Start with the Pokemon with smallest ID
        int smallestId = pokemonMap.begin()->first;
        std::set<int> ownedPokemon;
        ownedPokemon.insert(smallestId);
        
        bool newPokemonCaught = true;
        
        while (newPokemonCaught) {
            newPokemonCaught = false;
            
            for (const auto& pair : pokemonMap) {
                int targetId = pair.first;
                const Pokemon& targetPokemon = pair.second;
                
                // Skip if we already own this Pokemon
                if (ownedPokemon.find(targetId) != ownedPokemon.end()) {
                    continue;
                }
                
                // Check if any owned Pokemon can catch this one
                bool canCatch = false;
                for (int ownedId : ownedPokemon) {
                    const Pokemon& ownedPokemon = pokemonMap.at(ownedId);
                    
                    // Try each type of the owned Pokemon
                    for (const auto& attackType : ownedPokemon.types) {
                        float damage = attack(attackType.c_str(), targetId);
                        if (damage >= 2.0f) {
                            canCatch = true;
                            break;
                        }
                    }
                    if (canCatch) break;
                }
                
                if (canCatch) {
                    ownedPokemon.insert(targetId);
                    newPokemonCaught = true;
                }
            }
        }
        
        return ownedPokemon.size();
    }

    struct iterator {
    private:
        typename std::map<int, Pokemon>::iterator mapIt;
        typename std::map<int, Pokemon>::iterator mapEnd;
        
    public:
        iterator() {}
        
        iterator(typename std::map<int, Pokemon>::iterator it, typename std::map<int, Pokemon>::iterator end) 
            : mapIt(it), mapEnd(end) {}
        
        iterator &operator++() {
            if (mapIt != mapEnd) {
                ++mapIt;
            }
            return *this;
        }
        
        iterator &operator--() {
            if (mapIt != mapEnd) {
                --mapIt;
            }
            return *this;
        }
        
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        
        iterator & operator = (const iterator &rhs) {
            mapIt = rhs.mapIt;
            mapEnd = rhs.mapEnd;
            return *this;
        }
        
        bool operator == (const iterator &rhs) const {
            return mapIt == rhs.mapIt;
        }
        
        bool operator != (const iterator &rhs) const {
            return mapIt != rhs.mapIt;
        }
        
        Pokemon & operator*() const {
            if (mapIt == mapEnd) {
                throw IteratorException("Iterator Error: Invalid Iterator");
            }
            return const_cast<Pokemon&>(mapIt->second);
        }
        
        Pokemon *operator->() const {
            if (mapIt == mapEnd) {
                throw IteratorException("Iterator Error: Invalid Iterator");
            }
            return const_cast<Pokemon*>(&mapIt->second);
        }
    };

    iterator begin() {
        return iterator(pokemonMap.begin(), pokemonMap.end());
    }

    iterator end() {
        return iterator(pokemonMap.end(), pokemonMap.end());
    }
};