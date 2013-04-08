import dump_tileset

level_info = [
    (0xC6, 'STRT'),
    (0xC8, 'GR8T'),
    (0xCA, 'TLPT'),
    (0xCC, 'GRND'),
    (0x28, 'LLM0'),
    (0x2A, 'FL0T'),
    (0x2C, 'TRSS'),
    (0x2E, 'PRHS'),
    (0x30, 'CVRN'),
    (0x32, 'BBLS'),
    (0x34, 'VLCN'),
    (0x4B, 'QCKS'),
    (0x4D, 'PHR0'),
    (0x4F, 'C1R0'),
    (0x51, 'SPKS'),
    (0x53, 'JMNN'),
    (0x55, 'TTRS'),
    (0x72, 'JLLY'),
    (0x74, 'PLNG'),
    (0x76, 'BTRY'),
    (0x78, 'JNKR'),
    (0x7A, 'CBLT'),
    (0x7C, 'H0PP'),
    (0x7E, 'SMRT'),
    (0x80, 'V8TR'),
    (0x9C, 'NFL8'),
    (0x9E, 'WKYY'),
    (0xA0, 'CMB0'),
    (0xA2, '8BLL'),
    (0xA4, 'TRDR'),
    (0xA6, 'FNTM'),
    (0xA8, 'WRLR'),
    (0xAA, 'TRPD'),
    (0xCE, 'TFFF'),
    (0xD0, 'FRGT'),
    (0xD2, '4RN4'),
    (0xD4, 'MSTR'),
    (0x171, 'Respawn'),
    #(0x17D, 'MainMenu'),
    (0x186, 'Interplay'),
    (0x18C, 'SiliconSinapse'),
    (0x192, 'Warp'),
    #(0x17E, 'MainMenu2'),
    (0x1AF, 'Intro1'),
    (0x1B0, 'Intro2'),
    #(0x1B1, 'Ending1'),
    (0x1B2, 'Ending2'),
    (0xDA, 'Credits')
]

if __name__ == '__main__':
    filename = "tilesets/%02d_%s.png"
    for i, (level_id, name) in enumerate(level_info):
        print "Dumping %s..." % (name,)
        dump_tileset.main(level_id, filename % (i, name), 16)
