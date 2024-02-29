ifdef VERBOSE
Q :=
ECHO := @true
else
Q := @
ECHO := @echo
endif

check_variable_list = \
    $(strip $(foreach 1,$1, \
        $(call __check_variable_list,$1,$(strip $(value 2)))))
__check_variable_list = \
    $(if $(value $1),, \
      $(error Undefined $1$(if $2, ($2))))
