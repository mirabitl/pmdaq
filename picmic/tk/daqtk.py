#!/usr/bin/env python3


import json
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import ttkbootstrap as tb
from ttkbootstrap.constants import *
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import picmic_daq as pd
import picmic_register_access as cra
import transitions

data = {}
current_file = None
config_list = []
sdb=cra.instance()

daq=None

def update_daq_info():
    global daq
    
    if daq!=None:
        st=daq.get_status()
        state_var.set(f"State: {daq.state}")
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
    #popup.pack(expand="True")
    tab_list = ttk.Frame(popup)
    tab_list.pack(fill="both", expand=True)

    # On définit un layout en grille 2 colonnes
    tab_list.columnconfigure(0, weight=0)  # Colonne gauche : fixe
    tab_list.columnconfigure(1, weight=0)  # Colonne droite : prend l'espace
    tab_list.rowconfigure(0, weight=1)
    #
    # === 1. LabelFrame de boutons 1 (haut gauche) ===
    frame_list = ttk.LabelFrame(tab_list, text="Values")
    frame_list.grid(row=0, column=1, sticky="nsew", padx=5, pady=5)
    frame_act = ttk.LabelFrame(tab_list, text="Actions")
    frame_act.grid(row=0, column=0, sticky="nsew", padx=5, pady=5)

    #ttk.Label(popup, text=f"Liste : {path[-1]}").pack()

    listbox = tk.Listbox(frame_list)
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

    entry = ttk.Entry(frame_list)
    entry.pack(fill="x", padx=10, pady=5)

    ttk.Button(frame_act, text="Ajouter", bootstyle=SUCCESS,
               command=lambda: (liste.append(convert_type(entry.get())),
                                entry.delete(0, tk.END), refresh())).pack(pady=5)
    ttk.Button(frame_act, text="Modifier",
               command=lambda: (liste.__setitem__(listbox.curselection()[0], convert_type(entry.get())),
                                entry.delete(0, tk.END), refresh())).pack(pady=5)
    ttk.Button(frame_act, text="Supprimer",
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


def appliquer_modifs():
    global current_file
    global daq
    with open("/tmp/currentdaq.json", "w", encoding="utf-8") as f:
        json.dump(data, f, indent=4, ensure_ascii=False)
    #daq=pd.load_from_file("/tmp/currentdaq.json")
    if (daq==None):
        daq=pd.picmic_normal_run()
    daq.set_configuration(data)
    messagebox.showinfo("OK", "Tree view dat used,\n new picmic_daq set ✔")
    log(f"DAQ settings applied and saved in  →/tmp/currentdaq.json")
    

def ouvrir_json():
    global data, current_file
    fichier = filedialog.askopenfilename(filetypes=[("JSON", "*.json")])
    if fichier:
        with open(fichier, "r", encoding="utf-8") as f:
            data = json.load(f)
        current_file = fichier
        afficher_treeview()
        #maj_visualisation("JSON chargé")
        log(f"Chargé : {fichier}")


def ouvrir_json_file(fichier):
    global data, current_file

    if fichier:
        with open(fichier, "r", encoding="utf-8") as f:
            data = json.load(f)
        current_file = fichier
        afficher_treeview()
        #maj_visualisation("JSON chargé")
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
        try:
            daq.initialise()
            log(txt)
        except transitions.core.MachineError as e:
            messagebox.showerror("Wrong transition", f"{e} ✔")  
    else:
        messagebox.showerror("NO daq","DAQ is not created\n Please choose,load a configuration and Apply first✔")  
        log("initialise failed")

def bouton_configure():
    global daq
    txt = f"Configure, i.e prepare_run send"
    if daq!=None:
        try:
            daq.configure()
            log(txt)
        except transitions.core.MachineError as e:
            messagebox.showerror("Wrong transition", f"{e} ✔")  
    else:
        messagebox.showerror("NO daq","DAQ is not created\n Please choose,load a configuration and Apply first✔")
        log("configure failed")


def bouton_start():
    global daq
    txt = f"Start send"
    if daq!=None:
        try:
            daq.start()
            log(txt)
        except transitions.core.MachineError as e:
            messagebox.showerror("Wrong transition", f"{e} ✔")  

    else:
        messagebox.showerror("NO daq","DAQ is not created\n Please choose,load a configuration and Apply first✔")  
        log("start failed")
    
def bouton_stop():
    global daq
    txt = f"Stop send"
    if daq!=None:
        try:
            daq.stop()
            log(txt)
        except transitions.core.MachineError as e:
           messagebox.showerror("Wrong transition", f"{e} ✔")  
        
    else:
        messagebox.showerror("NO daq","DAQ is not created\n Please choose,load a configuration and Apply first✔")  
        log("stop failed")
        

def charger_config_mongo():
    global config_list

    config_list = sdb.configurations()

    if not config_list:
        messagebox.showwarning("Mongo", "Aucune configuration trouvée")
        return

    # Remplissage des noms dans le menu
    names = [f"{x[0]}:{x[1]}" for x in config_list]

    config_combo["values"] = names
    config_combo.config(state="readonly")
    config_combo.set("Sélectionnez une configuration…")

    log("→ Liste configurations Mongo chargée ✔")

def telecharger_config_selectionnee():
    if not config_combo.get():
        return
    
    index = config_combo.current()
    if index < 0:
        return

    print(config_list[index])

    config_name = config_list[index][0]
    config_version = int(config_list[index][1])

    temp_file = f"/dev/shm/config/{config_name}_{config_version}.json"
    sdb.download_configuration(config_name, config_version)

    log(f"→ Configuration Mongo téléchargée : {config_name}")
    ouvrir_json_file(temp_file)

# ----------------- UI ----------------- #
root = tb.Window()#themename="cyborg")
root.title("DAQ LIROC-PICOTDC 🧩")
root.geometry("800x650")

notebook = ttk.Notebook(root)
notebook.pack(fill="both", expand=True)

# --- Onglet JSON Viewer --- #
tab_json = ttk.Frame(notebook)
tab_json.pack(fill="both", expand=True)
notebook.add(tab_json, text="Configuration")
# On définit un layout en grille 2 colonnes
tab_json.columnconfigure(0, weight=0)  # Colonne gauche : fixe
tab_json.columnconfigure(1, weight=1)  # Colonne droite : prend l'espace
tab_json.rowconfigure(0, weight=1)
#
# === 1. LabelFrame de boutons 1 (haut gauche) ===
frame_file = ttk.LabelFrame(tab_json, text="Files")
frame_file.grid(row=0, column=0, sticky="nsew", padx=5, pady=5)



ttk.Button(frame_file, text="📂 Choisir la configuration JSON", bootstyle=PRIMARY,
           command=ouvrir_json).pack(side="bottom", padx=5, pady=5)
ttk.Button(frame_file, text="💾 Sauver dans un fichier", bootstyle=SUCCESS,
           command=enregistrer_modifs).pack(side="bottom", padx=5)

ttk.Button(tab_json, text="Apply and Create DAQ access(if needed)", bootstyle=SUCCESS,
           command=appliquer_modifs).grid(row=2,column=0, sticky="nsew",padx=10, pady=10)
#pack(side="bottom", padx=5)
### LabelFrame de boutons 2 (bas gauche) 
frame_db = ttk.LabelFrame(tab_json, text="DB")
frame_db.grid(row=1, column=0, sticky="nsew", padx=5, pady=5)

btn_mongo = ttk.Button(frame_db, text="Liste Configurations", bootstyle=INFO,
                       command=lambda: charger_config_mongo())
btn_mongo.pack(side="top", padx=5)

# Donne un minimum de hauteur aux frames de gauche
tab_json.rowconfigure(0, weight=0)
tab_json.rowconfigure(1, weight=0)
tab_json.rowconfigure(2, weight=0)


# === 3. LabelFrame droite : Treeview ===
lf_tree = ttk.LabelFrame(tab_json, text="Données JSON")
lf_tree.grid(row=0, column=1, rowspan=2, sticky="nsew", padx=5, pady=5)

tree = ttk.Treeview(lf_tree, columns=("value",), show="tree headings")
tree.heading("#0", text="Clé")
tree.heading("value", text="Valeur")
tree.pack(fill="both", expand=True)
#tree.grid(row=0,column=2, padx=10, pady=10)
#pack(fill="both", expand=True, padx=10, pady=10)
tree.bind("<Double-1>", edit_popup)

# --- Menu déroulant Configurations MongoDB --- #
#config_frame = ttk.Frame(tab_json)
#config_frame.grid(row=0,column=1, padx=10, pady=10)#pack(fill="x", padx=10, pady=5)

ttk.Label(frame_db, text="Configurations Mongo :").pack(side="top", padx=5)

config_var = tk.StringVar()
config_combo = ttk.Combobox(frame_db, textvariable=config_var, state="disabled")
config_combo.pack(side="top", fill="x", expand=True, padx=5)

btn_load_cfg = ttk.Button(frame_db, text="Charger",
                          bootstyle=INFO,
                          command=lambda: telecharger_config_selectionnee())
btn_load_cfg.pack(side="top", padx=5)

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

state_var=tk.StringVar(value="State: Unknown")
run_var = tk.StringVar(value="Run: 0")
event_var = tk.StringVar(value="Evt: 0")
status_var = tk.StringVar(value="Status: WAIT")

ttk.Label(daq_frame, textvariable=state_var, font=("Arial", 15, "bold")).pack(anchor="w", padx=8, pady=2)
ttk.Label(daq_frame, textvariable=run_var, font=("Arial", 12)).pack(anchor="w", padx=8, pady=2)
ttk.Label(daq_frame, textvariable=event_var, font=("Arial", 12)).pack(anchor="w", padx=8)
ttk.Label(daq_frame, textvariable=status_var, font=("Arial", 12, "italic")).pack(anchor="w", padx=8)

visu_frame = ttk.LabelFrame(tab_visu, text="Affichage")
visu_frame.pack(fill="both", expand=True, padx=10, pady=10)

#visu_label = ttk.Label(visu_frame, text="En attente d'une action...",
#                       font=("Arial", 16))
#visu_label.pack(expand=True)

# --- Onglet Logs --- #
#tab_logs = ttk.Frame(notebook)
#notebook.add(tab_logs, text="Logs")
log_text = tk.Text(visu_frame)#tab_logs)
log_text.pack(fill="both", expand=True)

log("Application prête 🚀")
# Log apps
tab_tail = ttk.Frame(notebook)
notebook.add(tab_tail, text="DAQ log file")

# Sélection du fichier
#file_path = filedialog.askopenfilename(title="Choisir un fichier à suivre")
file_path="/tmp/daq.log"
if file_path:
    tail_panel = TailFPanel(tab_tail, file_path, update_interval=1000)

update_daq_info()
root.mainloop()
