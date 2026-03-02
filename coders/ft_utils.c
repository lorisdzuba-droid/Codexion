#include "codexion.h"

long long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long long)tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	log_action(t_sim *sim, int coder_id, char *action)
{
	long long	timestamp;

	timestamp = get_time_ms() - sim->start_time;
	pthread_mutex_lock(&sim->print_mutex);
	printf("%lld %d %s\n", timestamp, coder_id, action);
	pthread_mutex_unlock(&sim->print_mutex);
}