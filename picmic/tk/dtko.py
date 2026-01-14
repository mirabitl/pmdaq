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
import picmic_scurve as ps
from datetime import datetime
import queue
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


class daq_widget:
    def __init__(self):
        self.data = {}
        self.current_file = None
        self.config_list = []
        self.sdb=cra.instance()
        self.daq=None
        self.calib_daq=None
        self.canvas_graph = None  # référence globale
        self.plot_canvas=None
        self.queue=queue.Queue()
    def get_json_ref(self,path):
        ref = self.data
        for key in path:
            if key.startswith("[") and key.endswith("]"):
                ref = ref[int(key[1:-1])]
            else:
                ref = ref[key]
        return ref

    def set_json_value(self,path, new_value):
        ref = self.data
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
    def get_item_path(self,item):
        path = []
        while item:
            path.insert(0, self.tree.item(item, "text"))
            item = self.tree.parent(item)
        return path
    # -------- LIST MODIFICATION POPUP -------- #
    def popup_liste(self,path, liste):
        popup = tk.Toplevel(self.root)
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
            self.set_json_value(path, liste)
            self.afficher_treeview()
            listbox.delete(0, tk.END)
            for i in liste:
                listbox.insert(tk.END, str(i))
            self.log(f"Liste modifiée : {path}")

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
    def edit_popup(self,event):
        selected = self.tree.selection()
        if not selected:
            return

        item = selected[0]
        path = self.get_item_path(item)
        current = self.get_json_ref(path)

        if isinstance(current, list):
            self.popup_liste(path, current)
            return
        if isinstance(current, dict):
            messagebox.showinfo("Info", "Edition dictionnaire bientôt disponible 😄")
            return

        popup = tk.Toplevel(self.root)
        popup.title("Modification")
        popup.geometry("350x150")

        ttk.Label(popup, text=f"Modifier {path[-1]}").pack(pady=10)
        entry = ttk.Entry(popup)
        entry.insert(0, str(current))
        entry.pack()

        def ok():
            self.set_json_value(path, convert_type(entry.get()))
            self.afficher_treeview()
            self.log(f"Valeur modifiée : {path}")
            popup.destroy()

        ttk.Button(popup, text="OK", bootstyle=INFO, command=ok).pack(pady=10)

    # ----------------- SAVE / LOAD ----------------- #
    def enregistrer_modifs(self):
        if self.current_file:
            rep = messagebox.askyesnocancel("Sauvegarder",
                                            "Sauver dans le même fichier ?")
            if rep is None:
                return
            if rep is False:
                self.current_file = filedialog.asksaveasfilename(
                    defaultextension=".json",
                    filetypes=[("JSON", "*.json")]
                )
        else:
            self.current_file = filedialog.asksaveasfilename(
                defaultextension=".json",
                filetypes=[("JSON", "*.json")]
            )

        if not self.current_file:
            return

        with open(self.current_file, "w", encoding="utf-8") as f:
            json.dump(self.data, f, indent=4, ensure_ascii=False)


    def appliquer_modifs(self):
        with open("/tmp/currentdaq.json", "w", encoding="utf-8") as f:
            json.dump(self.data, f, indent=4, ensure_ascii=False)
        #daq=pd.load_from_file("/tmp/currentdaq.json")
        if not "calibration" in self.data:
            if (self.daq==None): 
                self.daq=pd.picmic_normal_run()
            self.daq.set_configuration(self.data)
            messagebox.showinfo("OK", "Tree view dat used,\n new picmic_daq set ✔")
            self.log(f"DAQ settings applied and saved in  →/tmp/currentdaq.json")
        else:
            #self.calib_daq=ps.scurve_processor(self.data)
            messagebox.showinfo("OK", "Tree view dat used,\n new calib_scurve set ✔")
            self.log(f"Calibration settings applied and saved in  →/tmp/currentdaq.json")

    def ouvrir_json(self):
        fichier = filedialog.askopenfilename(filetypes=[("JSON", "*.json")])
        if fichier:
            with open(fichier, "r", encoding="utf-8") as f:
                self.data = json.load(f)
            self.current_file = fichier
            self.afficher_treeview()
            #maj_visualisation("JSON chargé")
            self.log(f"Chargé : {fichier}")


    def ouvrir_json_file(self,fichier):

        if fichier:
            with open(fichier, "r", encoding="utf-8") as f:
                self.data = json.load(f)
            self.current_file = fichier
            self.afficher_treeview()
            #maj_visualisation("JSON chargé")
            self.log(f"Chargé : {fichier}")

    # ----------------- TREEVIEW ----------------- #
    def remplir_tree(self,parent, element):
        if isinstance(element, dict):
            for k, v in element.items():
                item = self.tree.insert(parent, "end", text=k)
                self.remplir_tree(item, v)
        elif isinstance(element, list):
            for i, v in enumerate(element):
                item = self.tree.insert(parent, "end", text=f"[{i}]")
                self.remplir_tree(item, v)
        else:
            self.tree.insert(parent, "end", text=str(element))

    def afficher_treeview(self):
        self.tree.delete(*self.tree.get_children())
        self.remplir_tree("", self.data)


    def initialise_tab_json(self):
        # --- Onglet JSON Viewer --- #
        self.tab_json = ttk.Frame(self.notebook)
        self.tab_json.pack(fill="both", expand=True)
        self.notebook.add(self.tab_json, text="Configurations")
        # On définit un layout en grille 2 colonnes
        self.tab_json.columnconfigure(0, weight=0)  # Colonne gauche : fixe
        self.tab_json.columnconfigure(1, weight=1)  # Colonne droite : prend l'espace
        self.tab_json.rowconfigure(0, weight=1)
        #
        # === 1. LabelFrame de boutons 1 (haut gauche) ===
        self.frame_file = ttk.LabelFrame(self.tab_json, text="Files")
        self.frame_file.grid(row=0, column=0, sticky="nsew", padx=5, pady=5)
        ttk.Button(self.frame_file, text="📂 Choisir la configuration JSON", bootstyle=PRIMARY,
                   command=self.ouvrir_json).pack(side="bottom", padx=5, pady=5)
        ttk.Button(self.frame_file, text="💾 Sauver dans un fichier", bootstyle=SUCCESS,
                   command=self.enregistrer_modifs).pack(side="bottom", padx=5)

        ttk.Button(self.tab_json, text="Apply and Create DAQ access(if needed)", bootstyle=SUCCESS,
                   command=self.appliquer_modifs).grid(row=2,column=0, sticky="nsew",padx=10, pady=10)
        #pack(side="bottom", padx=5)
        ### LabelFrame de boutons 2 (bas gauche) 
        self.frame_db = ttk.LabelFrame(self.tab_json, text="DB")
        self.frame_db.grid(row=1, column=0, sticky="nsew", padx=5, pady=5)

        self.btn_mongo = ttk.Button(self.frame_db, text="Liste Configurations", bootstyle=INFO,
                               command=lambda: self.charger_config_mongo())
        self.btn_mongo.pack(side="top", padx=5)

        # Donne un minimum de hauteur aux frames de gauche
        self.tab_json.rowconfigure(0, weight=0)
        self.tab_json.rowconfigure(1, weight=0)
        self.tab_json.rowconfigure(2, weight=0)


        # === 3. LabelFrame droite : Treeview ===
        self.lf_tree = ttk.LabelFrame(self.tab_json, text="Données JSON")
        self.lf_tree.grid(row=0, column=1, rowspan=2, sticky="nsew", padx=5, pady=5)

        self.tree = ttk.Treeview(self.lf_tree, columns=("value",), show="tree headings")
        self.tree.heading("#0", text="Clé")
        self.tree.heading("value", text="Valeur")
        self.tree.pack(fill="both", expand=True)
        #tree.grid(row=0,column=2, padx=10, pady=10)
        #pack(fill="both", expand=True, padx=10, pady=10)
        self.tree.bind("<Double-1>", self.edit_popup)

        # --- Menu déroulant Configurations MongoDB --- #
        #config_frame = ttk.Frame(tab_json)
        #config_frame.grid(row=0,column=1, padx=10, pady=10)#pack(fill="x", padx=10, pady=5)

        ttk.Label(self.frame_db, text="Configurations Mongo :").pack(side="top", padx=5)

        self. config_var = tk.StringVar()
        self.config_combo = ttk.Combobox(self.frame_db, textvariable=self.config_var, state="disabled")
        self.config_combo.pack(side="top", fill="x", expand=True, padx=5)

        self.btn_load_cfg = ttk.Button(self.frame_db, text="Charger",
                                  bootstyle=INFO,
                                  command=lambda: self.telecharger_config_selectionnee())
        self.btn_load_cfg.pack(side="top", padx=5)

    def initialise_tab_run(self):
        self.tab_visu = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_visu, text="RUN PANEL")


        # 3 colonnes : boutons (fixe) | daq (fixe) | visu/plot (expand)
        self.tab_visu.columnconfigure(0, weight=0)
        self.tab_visu.columnconfigure(1, weight=0)
        self.tab_visu.columnconfigure(2, weight=1)

        # 2 lignes pour visu_frame et plot_frame
        self.tab_visu.rowconfigure(0, weight=1)
        self.tab_visu.rowconfigure(1, weight=1)
        #
        self.frame_btns = ttk.Frame(self.tab_visu)
        #self.frame_btns.pack(side="left", fill="y", padx=10, pady=10)
        self.frame_btns.grid(row=0, column=0, rowspan=2, sticky="ns", padx=10, pady=10)
        #for i in range(1, 1):
        ttk.Button(self.frame_btns, text=f"Calibration", bootstyle=WARNING,
                       command=lambda : self.bouton_calibration()).pack(pady=5)
        ttk.Label(self.frame_btns, text="Normal DAQ", font=("Arial", 15, "bold")).pack(pady=5)
        ttk.Button(self.frame_btns, text=f"Initialise", bootstyle=WARNING,
                   command= lambda :self.bouton_initialise() ).pack(pady=5)
        ttk.Button(self.frame_btns, text=f"Configure", bootstyle=WARNING,
                   command=lambda :self.bouton_configure() ).pack(pady=5)
        ttk.Button(self.frame_btns, text=f"Start", bootstyle=WARNING,
                   command=lambda :self.bouton_start()).pack(pady=5)
        ttk.Button(self.frame_btns, text=f"Stop", bootstyle=WARNING,
                   command=lambda :self.bouton_stop()).pack(pady=5)

        # --- Encart informations DAQ --- #
        self.daq_frame = ttk.LabelFrame(self.tab_visu, text="DAQ Info")
        #self.daq_frame.pack(side="left", fill="both", padx=10, pady=10)
        self.daq_frame.grid(row=0, column=1, rowspan=2, sticky="ns", padx=10, pady=10)
        self.state_var=tk.StringVar(value="State: Unknown")
        self.run_var = tk.StringVar(value="Run: 0")
        self.event_var = tk.StringVar(value="Evt: 0")
        self.status_var = tk.StringVar(value="Status: WAIT")

        ttk.Label(self.daq_frame, textvariable=self.state_var, font=("Arial", 15, "bold")).pack(anchor="w", padx=8, pady=2)
        ttk.Label(self.daq_frame, textvariable=self.run_var, font=("Arial", 12)).pack(anchor="w", padx=8, pady=2)
        ttk.Label(self.daq_frame, textvariable=self.event_var, font=("Arial", 12)).pack(anchor="w", padx=8)
        ttk.Label(self.daq_frame, textvariable=self.status_var, font=("Arial", 12, "italic")).pack(anchor="w", padx=8)

        self.visu_frame = ttk.LabelFrame(self.tab_visu, text="Commandes")
        self.visu_frame.grid(row=0, column=2, sticky="nsew", padx=10, pady=10)

        self.visu_frame.columnconfigure(0, weight=1)
        self.visu_frame.rowconfigure(0, weight=1)
        #self.visu_frame.pack(side="top",fill="both", expand=True, padx=10, pady=10)
        #self.visu_frame.columnconfigure(0, weight=0)  # Colonne droite : prend l'espace
        #self.visu_frame.rowconfigure(0, weight=0)
        self.log_text = tk.Text(self.visu_frame,height=5)#tab_logs)
        self.log_text.grid(row=0, column=0, sticky="nesw")

        self.plot_frame = ttk.LabelFrame(self.tab_visu, text="Plots")
        #self.plot_frame.pack(side="bottom",fill="both", expand=True, padx=10, pady=10)

        self.plot_frame.grid(row=1, column=2, sticky="nsew", padx=10, pady=10)
        self.plot_frame.grid_propagate(False)
        self.plot_frame.configure(height=400)

        # --- Onglet Logs --- #
        #tab_logs = ttk.Frame(notebook)
        #notebook.add(tab_logs, text="Logs")
        #self.log_text.pack(anchor="n", expand=False)

        self.log("Application prête 🚀")
    # ----------------- VISU / BUTTONS ----------------- #
    def maj_visualisation(self,txt):
        self.visu_label.config(text=txt)

    def clear_visu(self):
        if self.plot_canvas:
            self.plot_canvas.get_tk_widget().destroy()
            self.plot_canvas = None


    def afficher_graphique(self):
        pass
        self.clear_visu()  # on efface avant d'afficher un graph

        self.fig = Figure(figsize=(5, 4))
        ax = self.fig.add_subplot(111)
        ax.plot([1, 3, 2, 4, 6, 5])  # exemple
        ax.set_title("Graphique Matplotlib")

        self.canvas_graph = FigureCanvasTkAgg(self.fig, master=self.plot_frame)
        self.canvas_graph.draw()
        self.canvas_graph.get_tk_widget().pack(expand=True, fill="both")

        self.log("→ Affichage d'un graphique matplotlib")

    def bouton_calibration(self):
        #reinit plots
        self.clear_visu()
        self.plot_fig = Figure(figsize=(5,4), dpi=100)
        self.plot_canvas = FigureCanvasTkAgg(self.plot_fig, master=self.plot_frame)
        self.plot_canvas_widget = self.plot_canvas.get_tk_widget()
        self.plot_canvas_widget.pack(fill="both", expand=True)
        # Do calibration
        par=self.data
        if not "calibration" in par:
            messagebox.showerror("No calibration parameters", "Cannot call calibration ✔")
            return

        par["time"]=datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        with open(f'calib{par["time"]}.json', 'w') as fp:
            json.dump(par, fp,indent=2)

        self.scurve_process=ps.scurve_processor(par)
        self.scurve_process.queue = self.queue
        if par["calibration"] == "ALIGN":
            self.scurve_process.start_align()
        else:
            self.scurve_process.start_scurves(params={"analysis":par["calibration"],"plot_fig":self.plot_fig})
        

        #self.afficher_graphique()
        

    def bouton_initialise(self):

        txt = f"Initialise send"
        if self.daq!=None:
            try:
                self.daq.initialise()
                self.log(txt)
            except transitions.core.MachineError as e:
                messagebox.showerror("Wrong transition", f"{e} ✔")  
        else:
            messagebox.showerror("NO daq","DAQ is not created\n Please choose,load a configuration and Apply first✔")  
            self.log("initialise failed")

    def bouton_configure(self):

        txt = f"Configure, i.e prepare_run send"
        if self.daq!=None:
            try:
                self.daq.configure()
                self.log(txt)
            except transitions.core.MachineError as e:
                messagebox.showerror("Wrong transition", f"{e} ✔")  
        else:
            messagebox.showerror("NO daq","DAQ is not created\n Please choose,load a configuration and Apply first✔")
            self.log("configure failed")


    def bouton_start(self):

        txt = f"Start send"
        if self.daq!=None:
            try:
                self.daq.start()
                self.log(txt)
            except transitions.core.MachineError as e:
                messagebox.showerror("Wrong transition", f"{e} ✔")  

        else:
            messagebox.showerror("NO daq","DAQ is not created\n Please choose,load a configuration and Apply first✔")  
            self.log("start failed")

    def bouton_stop(self):

        txt = f"Stop send"
        if self.daq!=None:
            try:
                self.daq.stop()
                self.log(txt)
            except transitions.core.MachineError as e:
               messagebox.showerror("Wrong transition", f"{e} ✔")  

        else:
            messagebox.showerror("NO daq","DAQ is not created\n Please choose,load a configuration and Apply first✔")  
            self.log("stop failed")


        
    def initialise_ui(self):
        # ----------------- UI ----------------- #
        self.root = tb.Window()#themename="cyborg")
        self.root.title("DAQ LIROC-PICOTDC 🧩")
        self.root.geometry("800x650")

        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill="both", expand=True)
        # JSON config
        self.initialise_tab_json()
        # --- Onglet Visualisation + Boutons --- #
        self.initialise_tab_run()
        # Log apps
        self.tab_tail = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_tail, text="DAQ log file")

        # Sélection du fichier
        #file_path = filedialog.askopenfilename(title="Choisir un fichier à suivre")
        file_path="/tmp/daq.log"
        if file_path:
            self.tail_panel = TailFPanel(self.tab_tail, file_path, update_interval=1000)
        
    def update_daq_info(self):

        if self.daq!=None:
            st=self.daq.get_status()
            self.state_var.set(f"State: {self.daq.state}")
            self.run_var.set(f"Run: {st['run']}")
            self.event_var.set(f"Evt: {st['event']}")
            self.status_var.set("Status: RUNNING" if st['running'] else "Status: IDLE")
        if hasattr(self,'scurve_process') and self.scurve_process!=None:
            st=self.scurve_process.get_status()
            self.state_var.set(f"Method: {st.get('method','N/A')}")
            self.run_var.set(f"Target: {st.get('target','N/A')}")
            self.event_var.set(f"DAC Local: {st.get('dac_local','N/A')}")
            self.status_var.set("Status: RUNNING" if st.get('running',False) else "Status: IDLE")
        while not self.queue.empty():
            message = self.queue.get()
            if message == "update_plot":
                self.plot_canvas.draw_idle()  # Mettre à jour le plot dans le thread principal

        # Mise à jour toutes les 500 ms
        self.root.after(500, self.update_daq_info)
        
    def log(self,msg):
        self.log_text.insert(tk.END, msg + "\n")
        self.log_text.see(tk.END)

    def charger_config_mongo(self):

        self.config_list = self.sdb.configurations()

        if not self.config_list:
            messagebox.showwarning("Mongo", "Aucune configuration trouvée")
            return

        # Remplissage des noms dans le menu
        names = [f"{x[0]}:{x[1]}" for x in self.config_list]

        self.config_combo["values"] = names
        self.config_combo.config(state="readonly")
        self.config_combo.set("Sélectionnez une configuration…")

        self.log("→ Liste configurations Mongo chargée ✔")

    def telecharger_config_selectionnee(self):
        if not self.config_combo.get():
            return

        index = self.config_combo.current()
        if index < 0:
            return

        print(self.config_list[index])

        config_name = self.config_list[index][0]
        config_version = int(self.config_list[index][1])

        temp_file = f"/dev/shm/config/{config_name}_{config_version}.json"
        self.sdb.download_configuration(config_name, config_version)

        self.log(f"→ Configuration Mongo téléchargée : {config_name}")
        self.ouvrir_json_file(temp_file)

        
tkui=daq_widget()
tkui.initialise_ui()
tkui.update_daq_info()
tkui.root.mainloop()
