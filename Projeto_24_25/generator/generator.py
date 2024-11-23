from fpdf import FPDF

# Criar a classe PDF personalizada
class PDF(FPDF):
    def header(self):
        # Título da ficha
        self.set_font('Arial', 'B', 16)
        self.cell(0, 10, 'Ficha de Expressão Plástica – Natal', 0, 1, 'C')
        self.ln(5)

    def footer(self):
        # Número da página no rodapé
        self.set_y(-15)
        self.set_font('Arial', 'I', 12)
        self.cell(0, 10, f'Página {self.page_no()}', 0, 0, 'C')

    def add_title(self, title):
        self.set_font('Arial', 'B', 14)
        self.cell(0, 10, title, 0, 1)
        self.ln(2)

    def add_text(self, text):
        self.set_font('Arial', '', 12)
        self.multi_cell(0, 10, text)
        self.ln(4)

# Criar uma instância do PDF
pdf = PDF()
pdf.set_auto_page_break(auto=True, margin=15)
pdf.add_page()

# Adicionar o conteúdo à ficha
pdf.set_font('Arial', '', 12)
pdf.add_title("Nome do Aluno: _________________________  Data: ___/____/___")

# Conteúdo das atividades
activities = [
    ("1. Desenho Criativo de Natal",
     "Objetivo: Criar um desenho que represente o espírito natalino.\n"
     "Instruções:\n"
     "- Desenhe uma cena de")]