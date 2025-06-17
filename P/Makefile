# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -Iinclude

# Diretórios
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Encontra todos os arquivos .cpp na pasta src
SOURCES = $(wildcard $(SRCDIR)/*.cpp)

# Gera os nomes dos arquivos objeto .o a partir dos fontes .cpp
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

EXECUTABLE = $(BINDIR)/tp2.out

# Alvo padrão: compila tudo
all: $(EXECUTABLE)

# Regra para linkar o executável final a partir dos arquivos objeto
$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regra para compilar cada arquivo .cpp em um arquivo .o
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Limpa os arquivos gerados pela compilação
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Alvos "falsos" que não correspondem a arquivos
.PHONY: all clean
