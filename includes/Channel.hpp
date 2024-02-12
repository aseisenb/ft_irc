#ifndef CHANNELS_H
# define CHANNELS_H
# include "Struct.hpp"
# include <vector>

class Channel
{
	public:
		Channel();
		Channel(string name, int fd_creator);
		~Channel();

		bool			is_user(int	fd_user);
		bool			is_op(int fd_user);
		bool			is_invited(int fd_user);
		
		void			add_user(int fd_user);
		void			kick_user(int fd_to_kick);
		void			part(int fd_user);
		void			op_user( int fd_to_op);
		void			deop_user( int fd_to_deop);
		void			invite_user(int fd_to_invite);
		void			enable_locked_mode(string &key);
		void			disable_locked_mode();

		void			set_invite_only(bool mode);
		void			set_topic(string topic);
		void			unset_topic();
		void			set_protected_topic(bool mode);
		void			set_max_users(int max_users);
		void			set_has_user_limit(bool mode);


		string			get_name(void) const;
		vector<int>		get_users(void) const;
		vector<int>		get_ops(void) const;
		vector<int>		get_invited(void) const;
		string			get_topic(void) const;
		bool			get_invite_only(void) const;
		string			get_key(void) const;
		bool			get_topic_set(void) const;
		bool			get_topic_protected(void) const;
		bool			get_channel_locked(void) const;
		unsigned int	get_user_limit(void) const;
		bool			get_has_user_limit(void) const;
		void			broadcast(string message, int source);
		void			print_names(int target_fd);
		static Channel *getChannel(string name);

	private:
		string			_name;
		string			topic;
		string			key;
		vector<int>		user_fd;
		vector<int>		ops_fd;
		vector<int>		invited_fd;
		bool			invite_only;
		bool			topic_is_set;
		bool			topic_is_protected;
		bool			channel_is_locked;
		unsigned int	user_MAX;
		bool			has_user_limit;

};

#endif