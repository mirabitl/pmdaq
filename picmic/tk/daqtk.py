import json
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import ttkbootstrap as tb
from ttkbootstrap.constants import *
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import picmic_daq as pd

data = {}
current_file = None

daq=None
def update_daq_info():
    global daq
    
    if daq!=None:
        st=daq.get_status()
        run_var.set(f"Run: {st['run']}")
        event_var.set(f"Evt: {st['event']}")
        status_var.set("Status: RUNNING" if st['running'] else "Status: IDLE")

    # Mise à jour toutes les 500 ms
    root.after(500, update_daq_info)
class TailFPanel:
    def __init__(self, parent, file_path, update_interval=1000):
        self.parent = parent
        self.file_path = file_path
        self.update_interval = update_interval  # ms
        self.position = 0  # position du fichier

        # Text widget
        self.text = tk.Text(parent, height=20, width=80)
        self.text.pack(fill="both", expand=True)

        # Start
        self.update()

    def update(self):
        try:
            with open(self.file_path, "r") as f:
                f.seek(self.position)
                lines = f.readlines()
                if lines:
                    self.text.insert(tk.END, "".join(lines))
                    self.text.see(tk.END)  # scroll automatique
                    self.position = f.tell()
        except Exception as e:
            self.text.insert(tk.END, f"Erreur lecture fichier: {e}\n")
        
        self.parent.after(self.update_interval, self.update)

# ------------------------- LOGGING ------------------------- #
def log(msg):
    log_text.insert(tk.END, msg + "\n")
    log_text.see(tk.END)

# -------- TYPE CONVERSION -------- #
def convert_type(value: str):
    if value.isdigit():
        return int(value)
    try:
        return float(value)
    except ValueError:
        if value.lower() == "true":
            return True
        if value.lower() == "false":
            return False
        return value

# -------- JSON PATH FINDING -------- #
def get_item_path(item):
    path = []
    while item:
        path.insert(0, tree.item(item, "text"))
        item = tree.parent(item)
    return path

def get_json_ref(path):
    ref = data
    for key in path:
        if key.startswith("[") and key.endswith("]"):
            ref = ref[int(key[1:-1])]
        else:
            ref = ref[key]
    return ref

def set_json_value(path, new_value):
    ref = data
    for key in path[:-1]:
        if key.startswith("[") and key.endswith("]"):
            ref = ref[int(key[1:-1])]
        else:
            ref = ref[key]

    last = path[-1]
    if last.startswith("[") and last.endswith("]"):
        ref[int(last[1:-1])] = new_value
    else:
        ref[last] = new_value

# -------- LIST MODIFICATION POPUP -------- #
def popup_liste(path, liste):
    popup = tk.Toplevel(root)
    popup.title("Modifier une liste")
    popup.geometry("400x300")

    ttk.Label(popup, text=f"Liste : {path[-1]}").pack()

    listbox = tk.Listbox(popup)
    listbox.pack(fill="both", expand=True, padx=10, pady=10)

    def refresh():
        set_json_value(path, liste)
        afficher_treeview()
        listbox.delete(0, tk.END)
        for i in liste:
            listbox.insert(tk.END, str(i))
        log(f"Liste modifiée : {path}")

    for item in liste:
        listbox.insert(tk.END, str(item))

    entry = ttk.Entry(popup)
    entry.pack(fill="x", padx=10, pady=5)

    ttk.Button(popup, text="Ajouter", bootstyle=SUCCESS,
               command=lambda: (liste.append(convert_type(entry.get())),
                                entry.delete(0, tk.END), refresh())).pack(pady=5)
    ttk.Button(popup, text="Modifier",
               command=lambda: (liste.__setitem__(listbox.curselection()[0], convert_type(entry.get())),
                                entry.delete(0, tk.END), refresh())).pack(pady=5)
    ttk.Button(popup, text="Supprimer",
               command=lambda: (liste.pop(listbox.curselection()[0]), refresh())).pack(pady=5)

# -------- EDIT POPUP -------- #
def edit_popup(event):
    selected = tree.selection()
    if not selected:
        return

    item = selected[0]
    path = get_item_path(item)
    current = get_json_ref(path)

    if isinstance(current, list):
        popup_liste(path, current)
        return
    if isinstance(current, dict):
        messagebox.showinfo("Info", "Edition dictionnaire bientôt disponible 😄")
        return

    popup = tk.Toplevel(root)
    popup.title("Modification")
    popup.geometry("350x150")

    ttk.Label(popup, text=f"Modifier {path[-1]}").pack(pady=10)
    entry = ttk.Entry(popup)
    entry.insert(0, str(current))
    entry.pack()

    def ok():
        set_json_value(path, convert_type(entry.get()))
        afficher_treeview()
        log(f"Valeur modifiée : {path}")
        popup.destroy()

    ttk.Button(popup, text="OK", bootstyle=INFO, command=ok).pack(pady=10)

# ----------------- SAVE / LOAD ----------------- #
def enregistrer_modifs():
    global current_file
    global daq
    if current_file:
        rep = messagebox.askyesnocancel("Sauvegarder",
                                        "Sauver dans le même fichier ?")
        if rep is None:
            return
        if rep is False:
            current_file = filedialog.asksaveasfilename(
                defaultextension=".json",
                filetypes=[("JSON", "*.json")]
            )
    else:
        current_file = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON", "*.json")]
        )

    if not current_file:
        return

    with open(current_file, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=4, ensure_ascii=False)

    daq=pd.load_from_file(current_file)
    messagebox.showinfo("OK", "Enregistré ✔")
    log(f"Sauvegarde → {current_file}")

def ouvrir_json():
    global data, current_file
    fichier = filedialog.askopenfilename(filetypes=[("JSON", "*.json")])
    if fichier:
        with open(fichier, "r", encoding="utf-8") as f:
            data = json.load(f)
        current_file = fichier
        afficher_treeview()
        maj_visualisation("JSON chargé")
        log(f"Chargé : {fichier}")

# ----------------- TREEVIEW ----------------- #
def remplir_tree(parent, element):
    if isinstance(element, dict):
        for k, v in element.items():
            item = tree.insert(parent, "end", text=k)
            remplir_tree(item, v)
    elif isinstance(element, list):
        for i, v in enumerate(element):
            item = tree.insert(parent, "end", text=f"[{i}]")
            remplir_tree(item, v)
    else:
        tree.insert(parent, "end", text=str(element))

def afficher_treeview():
    tree.delete(*tree.get_children())
    remplir_tree("", data)

# ----------------- VISU / BUTTONS ----------------- #
def maj_visualisation(txt):
    visu_label.config(text=txt)


canvas_graph = None  # référence globale

def clear_visu():
    global canvas_graph
    if canvas_graph:
        canvas_graph.get_tk_widget().destroy()
        canvas_graph = None
    visu_label.config(text="")

def afficher_graphique():
    global canvas_graph

    clear_visu()  # on efface avant d'afficher un graph

    fig = Figure(figsize=(5, 4))
    ax = fig.add_subplot(111)
    ax.plot([1, 3, 2, 4, 6, 5])  # exemple
    ax.set_title("Graphique Matplotlib")

    canvas_graph = FigureCanvasTkAgg(fig, master=visu_frame)
    canvas_graph.draw()
    canvas_graph.get_tk_widget().pack(expand=True, fill="both")

    log("→ Affichage d'un graphique matplotlib")

def bouton_action(i):
    if i == 1:  # Exemple → bouton 1 affiche un graphique
        afficher_graphique()
    else:
        clear_visu()
        txt = f"Clic sur bouton {i}"
        visu_label.config(text=txt)
        log(txt)
    
def bouton_action_old(i):
    txt = f"Clic sur bouton {i}"
    maj_visualisation(txt)
    log(txt)

def bouton_initialise():
    global daq
    txt = f"Initialise send"
    if daq!=None:
        daq.initialise()
    log(txt)

def bouton_configure():
    global daq
    txt = f"Configure, i.e prepare_run send"
    if daq!=None:
        daq.prepare_run()
    log(txt)

def bouton_start():
    global daq
    txt = f"Start send"
    if daq!=None:
        daq.start_a_run()
    log(txt)
    
def bouton_stop():
    global daq
    txt = f"Start send"
    if daq!=None:
        daq.stop()
    log(txt)

# ----------------- UI ----------------- #
root = tb.Window(themename="cyborg")
root.title("DAQ LIROC-PICOTDC 🧩")
root.geometry("1100x650")

notebook = ttk.Notebook(root)
notebook.pack(fill="both", expand=True)

# --- Onglet JSON Viewer --- #
tab_json = ttk.Frame(notebook)
notebook.add(tab_json, text="Fichier de configuration")

ttk.Button(tab_json, text="📂 Choisir la configuration JSON", bootstyle=PRIMARY,
           command=ouvrir_json).pack(side="left", padx=5, pady=5)
ttk.Button(tab_json, text="💾 Appliquer et Sauver", bootstyle=SUCCESS,
           command=enregistrer_modifs).pack(side="left", padx=5)

tree = ttk.Treeview(tab_json)
tree.pack(fill="both", expand=True, padx=10, pady=10)
tree.bind("<Double-1>", edit_popup)

# --- Onglet Visualisation + Boutons --- #
tab_visu = ttk.Frame(notebook)
notebook.add(tab_visu, text="RUN PANEL")

frame_btns = ttk.Frame(tab_visu)
frame_btns.pack(side="left", fill="y", padx=10, pady=10)

for i in range(1, 1):
    ttk.Button(frame_btns, text=f"Bouton {i}", bootstyle=WARNING,
               command=lambda i=i: bouton_action(i)).pack(pady=5)

ttk.Button(frame_btns, text=f"Initialise", bootstyle=WARNING,
           command= lambda :bouton_initialise() ).pack(pady=5)
ttk.Button(frame_btns, text=f"Configure", bootstyle=WARNING,
           command=lambda :bouton_configure() ).pack(pady=5)
ttk.Button(frame_btns, text=f"Start", bootstyle=WARNING,
           command=lambda :bouton_start()).pack(pady=5)
ttk.Button(frame_btns, text=f"Stop", bootstyle=WARNING,
           command=lambda :bouton_stop()).pack(pady=5)

# --- Encart informations DAQ --- #
daq_frame = ttk.LabelFrame(tab_visu, text="DAQ Info")
daq_frame.pack(side="left", fill="both", padx=10, pady=10)

run_var = tk.StringVar(value="Run: 0")
event_var = tk.StringVar(value="Evt: 0")
status_var = tk.StringVar(value="Status: WAIT")

ttk.Label(daq_frame, textvariable=run_var, font=("Arial", 13, "bold")).pack(anchor="w", padx=8, pady=2)
ttk.Label(daq_frame, textvariable=event_var, font=("Arial", 12)).pack(anchor="w", padx=8)
ttk.Label(daq_frame, textvariable=status_var, font=("Arial", 11, "italic")).pack(anchor="w", padx=8)

visu_frame = ttk.LabelFrame(tab_visu, text="Affichage")
visu_frame.pack(fill="both", expand=True, padx=10, pady=10)

visu_label = ttk.Label(visu_frame, text="En attente d'une action...",
                       font=("Arial", 16))
visu_label.pack(expand=True)

# --- Onglet Logs --- #
tab_logs = ttk.Frame(notebook)
notebook.add(tab_logs, text="Logs")
log_text = tk.Text(tab_logs)
log_text.pack(fill="both", expand=True)

log("Application prête 🚀")
# Log apps
tab_tail = ttk.Frame(notebook)
notebook.add(tab_tail, text="Tail File")

# Sélection du fichier
#file_path = filedialog.askopenfilename(title="Choisir un fichier à suivre")
file_path="/tmp/daq.log"
if file_path:
    tail_panel = TailFPanel(tab_tail, file_path, update_interval=1000)

update_daq_info()
root.mainloop()
