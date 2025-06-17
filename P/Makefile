# 1. Variáveis do Compilador
CXX = g++
# Define o padrão do C++. C++17 é uma escolha moderna e segura.
CXX_STANDARD = -std=c++17

# 2. Flags de Compilação
# -Wall -Wextra -Wpedantic: Ativam um conjunto robusto de avisos para erros comuns.
# -Werror: Trata todos os avisos como erros, forçando a correção.
# -g: Adiciona símbolos de depuração (essencial para o GDB).
CXXFLAGS = -Wall -Wextra -Wpedantic -Werror $(CXX_STANDARD) -g

# 3. Estrutura de Diretórios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INCLUDE_DIR = -Iinclude

# Nome do executável final
EXECUTABLE = $(BIN_DIR)/tp2.out

# 4. Detecção Automática de Arquivos
# Encontra todos os arquivos .cpp na pasta src
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
# Gera os nomes dos arquivos-objeto correspondentes na pasta obj
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

# 5. Regras de Compilação (Targets)

# Regra principal: 'make' ou 'make all'
all: $(EXECUTABLE)

# Regra para linkar os arquivos-objeto e criar o executável
$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BIN_DIR) # Cria o diretório bin/ se ele não existir
	$(CXX) $(OBJECTS) -o $(EXECUTABLE)

# Regra para compilar cada arquivo .cpp em um arquivo .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR) # Cria o diretório obj/ se ele não existir
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIR) -c $< -o $@

# Regra para limpar os arquivos gerados
clean:
	rm -rf $(OBJ_DIR)

# Regra de limpeza completa (objetos e executável)
fclean: clean
	rm -f $(EXECUTABLE)

# Regra para recompilar tudo do zero
re: fclean all

# Declara alvos que não são arquivos
.PHONY: all clean fclean re