_CONVERTERS = {
    'WORKERS': int,
    'INEQUALITIES': int,
    'TIME': float}


def read_output_file(filename):
    ret = {}
    with open(filename, 'r') as f:
        for l in f.readlines():
            if '=' not in l:
                continue

            key, value = l.split('=')
            if key == 'FILENAME':
                input_filename = value.split('/')[-1]
                input_filename_parts = input_filename.split('__')
                ret['SIZE'] = int(input_filename_parts[-2])
                ret['SEED'] = int(input_filename_parts[-1][:-5])
            else:
                if key in _CONVERTERS:
                    ret[key] = _CONVERTERS[key](value)
    return ret
