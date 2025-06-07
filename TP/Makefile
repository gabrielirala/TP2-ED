# Configurações do compilador
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
INCLUDES = -Iinclude
LDFLAGS = 

# Diretórios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INCLUDE_DIR = include

# Nome do executável
TARGET = $(BIN_DIR)/simulador_logistico

# Encontrar todos os arquivos .cpp
SOURCES = $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Dependências automáticas
DEPS = $(OBJECTS:.o=.d)

# Regra principal
all: $(TARGET)

# Criação do executável
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Executável criado: $(TARGET)"

# Compilação dos objetos
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Criação dos diretórios
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Incluir dependências
-include $(DEPS)

# Limpeza
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Arquivos limpos"

# Limpeza completa
distclean: clean
	rm -f $(DEPS)

# Instalação (opcional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Desinstalação
uninstall:
	rm -f /usr/local/bin/simulador_logistico

# Executar testes
test: $(TARGET)
	./$(TARGET) --test

# Debug
debug: CXXFLAGS += -DDEBUG -g3 -O0
debug: $(TARGET)

# Release
release: CXXFLAGS += -DNDEBUG -O3
release: $(TARGET)

# Análise estática
analyze:
	cppcheck --enable=all --std=c++17 $(SRC_DIR)

# Formatação de código
format:
	find $(SRC_DIR) $(INCLUDE_DIR) -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i

# Documentação
docs:
	doxygen Doxyfile

# Ajuda
help:
	@echo "Targets disponíveis:"
	@echo "  all      - Compila o projeto (padrão)"
	@echo "  clean    - Remove arquivos compilados"
	@echo "  debug    - Compila versão de debug"
	@echo "  release  - Compila versão otimizada"
	@echo "  test     - Executa testes"
	@echo "  install  - Instala o executável"
	@echo "  analyze  - Análise estática do código"
	@echo "  format   - Formatar código"
	@echo "  docs     - Gerar documentação"

.PHONY: all clean distclean install uninstall test debug release analyze format docs help

# Configurações específicas do sistema
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    RM = del /Q
    MKDIR = mkdir
else
    RM = rm -f
    MKDIR = mkdir -p
endif

# Detecção de dependências do sistema
HAS_VALGRIND := $(shell command -v valgrind 2> /dev/null)
ifdef HAS_VALGRIND
memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)
endif

# Profiling
profile: CXXFLAGS += -pg
profile: $(TARGET)
	./$(TARGET)
	gprof $(TARGET) gmon.out > profile.txt
