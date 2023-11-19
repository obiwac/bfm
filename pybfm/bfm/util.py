from typing import List

def jsify_list(l: List):
	return "[" + ",".join(map(lambda x: str(round(x, 3)), l)) + "]"
