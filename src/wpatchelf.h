#if !defined(PATCHELF_H)
#define PATCHELF_H

#if defined(__cplusplus)
	#define export_c extern "C"
#else
	#define export_c
#endif

struct CPatchElf {
	void* data;
};

typedef struct CPatchElf patchelf_t;

export_c patchelf_t* patchelf_init(void);

export_c void patchelf_set_interpreter(
	patchelf_t* const patchelf,
	const char* const interpreter
);

export_c void patchelf_force_rpath(
	patchelf_t* const patchelf,
	const int force
);

export_c void patchelf_add_rpath(
	patchelf_t* const patchelf,
	const char* const path
);

export_c void patchelf_add_file(
	patchelf_t* const patchelf,
	const char* const path
);

export_c void patchelf_perform(patchelf_t* const patchelf);
export_c void patchelf_free(patchelf_t* const patchelf);

#if defined(__cplusplus)
class PatchElf {
	private:
		std::string rpath;
		std::string interpreter;
		std::vector<std::string> filenames;
		bool force_rpath;
	
	public:
		void set_interpreter(const std::string interpreter);
		void set_force_rpath(const bool force);
		void add_rpath(const std::string rpath);
		void add_file(const std::string path);
		void perform(void);
};

void PatchElf::set_interpreter(const std::string interpreter) {
	this->interpreter = interpreter;
}

void PatchElf::set_force_rpath(const bool force) {
	this->force_rpath = force;
}

void PatchElf::add_rpath(const std::string path) {
	
	if (!this->rpath.empty()) {
		this->rpath.append(":");
	}
	
	this->rpath.append(path);
	
}

void PatchElf::add_file(const std::string path) {
	
	this->filenames.push_back(path);
	
}

void PatchElf::perform(void) {
	
	fileNames.clear();
	
	setRPath = false;
	addRPath = false;
	forceRPath = false;
	newRPath = "";
	newInterpreter = "";
	
	if (this->filenames.empty()) {
		return;
	}
	
	if (!this->rpath.empty()) {
		setRPath = true;
		newRPath = this->rpath;
	}
	
	if (!this->interpreter.empty()) {
		newInterpreter = interpreter;
	}
	
	for (std::string item : this->filenames) {
		fileNames.push_back(item);
	}
	
	forceRPath = this->force_rpath;
	
	patchElf();
	
}

patchelf_t* patchelf_init(void) {
	
	patchelf_t* patchelf = new patchelf_t;
	patchelf->data = new PatchElf;
	
	return patchelf;
	
}

void patchelf_set_interpreter(
	patchelf_t* const patchelf,
	const char* const interpreter
) {
	
	const std::string a = std::string(interpreter);
	((PatchElf*) patchelf->data)->set_interpreter(a);
	
}

void patchelf_force_rpath(
	patchelf_t* const patchelf,
	const int force
) {
	
	((PatchElf*) patchelf->data)->set_force_rpath(force);
	
}

void patchelf_add_rpath(
	patchelf_t* const patchelf,
	const char* const path
) {
	
	const std::string a = std::string(path);
	((PatchElf*) patchelf->data)->add_rpath(a);
	
}

void patchelf_add_file(
	patchelf_t* const patchelf,
	const char* const path
) {
	
	const std::string a = std::string(path);
	((PatchElf*) patchelf->data)->add_file(a);
	
}

void patchelf_perform(patchelf_t* const patchelf) {
	((PatchElf*) patchelf->data)->perform();
}

void patchelf_free(patchelf_t* const patchelf) {
	
	if (patchelf == nullptr) {
		return;
	}
	
	delete ((PatchElf*) patchelf->data);
	delete (patchelf);
	
}

#endif
#endif /* PATCHELF_H */
