void dump_keys(const SSL*, const char* line) {
#ifdef DUMP_SSL_SECRETS
	static FILE* fp = fopen("secrets.txt", "a");
	fprintf(fp, "%s\n", line);
	fflush(fp);
#endif
}
