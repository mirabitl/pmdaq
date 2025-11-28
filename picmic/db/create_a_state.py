import picmic_register_access as pr
s=pr.instance()
s.list_setups()
s.create_setup("PICMIC_EXEMPLE_BOARD2")
pr=pr.picmic_registers(f_id=2)
pr.load_defaults(fnp="../etc/default_pico.csv",fnl="../etc/default_liroc.csv")
s.setup.add_picmic(pr)
# Set it in Fine Mode
s.setup.picmic.setResMode('fine')
s.upload_changes("un test de creation")
