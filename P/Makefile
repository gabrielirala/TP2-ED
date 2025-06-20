# Compilador e flags
CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic -Werror -std=c++17 -g -Iinclude

# Diretórios
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# Encontra todos os arquivos .cpp no diretório src
SOURCES = $(wildcard $(SRCDIR)/*.cpp)

# Gera os nomes dos arquivos objeto a partir dos fontes
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

# Nome do executável
EXECUTABLE = $(BINDIR)/tp2.out

# Regra principal: criar o executável
all: $(EXECUTABLE)

# Regra para linkar os objetos e criar o executável final
$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) -o $@ $^

# Regra para compilar cada arquivo .cpp em um arquivo .o
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Regra para criar o diretório de objetos, se não existir
$(OBJDIR):
	@mkdir -p $@

# Regra para limpar os arquivos gerados
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Phony targets não são nomes de arquivos
.PHONY: all clean