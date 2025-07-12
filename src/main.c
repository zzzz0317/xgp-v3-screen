// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 zzzz0317

#include "lvgl/lvgl.h"
// #include "lvgl/demos/lv_demos.h"
#include "ui/ui.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define UNKNOWN_VALUE_REPLACE_STRING "未知"
#define UNKNOWN_IP_REPLACE_STRING "无IP地址或接口不存在"
#define DEFAULT_VALUE_SIZE 64
#define MAX_ENV_LINE_LENGTH 128

#define MAX_IFACE_NAME_LEN 16
#define MAX_IP_ADDR_LEN 16

static const char *getenv_default(const char *name, const char *dflt)
{
    return getenv(name) ?: dflt;
}

static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t *disp = lv_linux_fbdev_create();

    lv_linux_fbdev_set_file(disp, device);
}

void format_memory_size(long bytes, char *buffer)
{
    const double MiB = 1024 * 1024;
    const double GiB = 1024 * 1024 * 1024;

    if (bytes >= GiB)
    {
        double gib = bytes / GiB;
        sprintf(buffer, "%.2fG", gib);
    }
    else
    {
        double mib = bytes / MiB;
        sprintf(buffer, "%.2fM", mib);
    }
}

bool extract_env_value(const char *line, const char *key, char *value, size_t value_size)
{
    size_t key_len = strlen(key);
    if (strncmp(line, key, key_len) != 0)
    {
        return false;
    }

    const char *equal_sign = strchr(line, '=');
    if (equal_sign == NULL)
    {
        return false;
    }

    const char *value_start = equal_sign + 1;

    if (*value_start == '"')
    {
        value_start++;
        const char *value_end = strchr(value_start, '"');
        if (value_end == NULL)
        {
            return false;
        }
        size_t value_len = value_end - value_start;
        if (value_len >= value_size)
        {
            value_len = value_size - 1;
        }
        strncpy(value, value_start, value_len);
        value[value_len] = '\0';
    }
    else
    {
        size_t value_len = strlen(value_start);
        if (value_len >= value_size)
        {
            value_len = value_size - 1;
        }
        strncpy(value, value_start, value_len);
        value[value_len] = '\0';
    }

    return true;
}

int read_os_release(char *pretty_name, size_t pretty_name_size,
                    char *build_id, size_t build_id_size)
{
    FILE *file = fopen("/etc/os-release", "r");
    if (file == NULL)
    {
        return -1;
    }

    bool found_pretty_name = false;
    bool found_build_id = false;
    char line[MAX_ENV_LINE_LENGTH];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';

        if (!found_pretty_name)
        {
            found_pretty_name = extract_env_value(line, "PRETTY_NAME",
                                                  pretty_name, pretty_name_size);
        }

        if (!found_build_id)
        {
            found_build_id = extract_env_value(line, "BUILD_ID",
                                               build_id, build_id_size);
        }

        if (found_pretty_name && found_build_id)
        {
            break;
        }
    }

    fclose(file);

    if (!found_pretty_name)
    {
        strncpy(pretty_name, UNKNOWN_VALUE_REPLACE_STRING, pretty_name_size);
    }

    if (!found_build_id)
    {
        strncpy(build_id, UNKNOWN_VALUE_REPLACE_STRING, build_id_size);
    }

    return 0;
}

int read_file_to_string(char *dest, size_t dest_size, const char *filename)
{
    if (dest == NULL || dest_size == 0 || filename == NULL)
    {
        return -1;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return -1;
    }

    size_t bytes_read = 0;
    int c;

    while (bytes_read < dest_size - 1 && (c = fgetc(file)) != EOF)
    {
        dest[bytes_read++] = (char)c;
    }

    dest[bytes_read] = '\0';

    fclose(file);
    return (int)bytes_read;
}

int get_interface_ipv4_address(const char *iface_name, char *ip_addr, size_t ip_addr_len)
{
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_in *sin;

    if (iface_name == NULL || ip_addr == NULL || ip_addr_len < MAX_IP_ADDR_LEN)
    {
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    strncpy(ifr.ifr_name, iface_name, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sockfd, SIOCGIFADDR, &ifr) == -1)
    {
        close(sockfd);
        return -1; // 接口不存在或没有IP地址
    }

    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    const char *ip = inet_ntoa(sin->sin_addr);

    if (ip != NULL)
    {
        strncpy(ip_addr, ip, ip_addr_len - 1);
        ip_addr[ip_addr_len - 1] = '\0';
        close(sockfd);
        return 0;
    }

    close(sockfd);
    return -1;
}

int get_first_wwan_ipv4_address(char *ip_addr, size_t ip_addr_len)
{
    struct if_nameindex *if_ni, *if_entry;

    if_ni = if_nameindex();
    if (if_ni == NULL)
    {
        perror("if_nameindex");
        return -1;
    }

    for (if_entry = if_ni; if_entry->if_index != 0; if_entry++)
    {
        // 检查是否是wwan接口 (wwan0, wwan1, wwan2等)
        if (strncmp(if_entry->if_name, "wwan", 4) == 0)
        {
            if (get_interface_ipv4_address(if_entry->if_name, ip_addr, ip_addr_len) == 0)
            {
                if_freenameindex(if_ni);
                return 0; // 成功找到wwan接口并获取IP
            }
        }
    }

    if_freenameindex(if_ni);
    return -1; // 没有找到有IP的wwan接口
}

int get_nf_conntrack_count()
{
    FILE *fp;
    char command[] = "wc -l /proc/net/nf_conntrack 2>/dev/null";
    char line[256];
    int count = -1;
    fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("popen failed");
        return -1;
    }
    if (fgets(line, sizeof(line), fp) != NULL)
    {
        char *token = strtok(line, " ");
        if (token != NULL)
        {
            count = atoi(token);
        }
    }
    pclose(fp);
    return count;
}

int count_arp_online()
{
    FILE *fp;
    char command[] = "cat /proc/net/arp";
    char line[256];
    int count = 0;
    bool is_header = true;

    fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("popen failed");
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (is_header)
        {
            is_header = false;
            continue;
        }

        char ip[16], hw_type[8], flags[8], hw_addr[18], mask[8], device[16];
        if (sscanf(line, "%15s %7s %7s %17s %7s %15s",
                   ip, hw_type, flags, hw_addr, mask, device) == 6)
        {
            if (strcmp(flags, "0x2") == 0)
            {
                count++;
            }
        }
    }

    pclose(fp);
    return count;
}

static uint32_t update_screen_data_time_counter = 0;
static uint32_t update_modem_data_time_counter = 10;

static long memory_total_bytes = 0;
static char buf_memory_total_bytes[DEFAULT_VALUE_SIZE];

static char buf_hostname[DEFAULT_VALUE_SIZE];
static char buf_sys_version[DEFAULT_VALUE_SIZE];
static char buf_build_id[DEFAULT_VALUE_SIZE];
static char buf_kernel_version[DEFAULT_VALUE_SIZE];
static char buf_load_avg[DEFAULT_VALUE_SIZE];
static char buf_memory[DEFAULT_VALUE_SIZE];
static char buf_uptime[DEFAULT_VALUE_SIZE];
static char buf_local_time[DEFAULT_VALUE_SIZE];
static char buf_modem_ip[DEFAULT_VALUE_SIZE];
static char buf_wan_ip[DEFAULT_VALUE_SIZE];
static char buf_lan_ip[DEFAULT_VALUE_SIZE];
static char buf_active_connect[DEFAULT_VALUE_SIZE];
static char buf_arp_count[DEFAULT_VALUE_SIZE];

static char buf_modem_revision[DEFAULT_VALUE_SIZE];
static char buf_modem_temperature[DEFAULT_VALUE_SIZE];
static char buf_modem_voltage[DEFAULT_VALUE_SIZE];
static char buf_modem_connect[DEFAULT_VALUE_SIZE];
static char buf_modem_sim[DEFAULT_VALUE_SIZE];
static char buf_modem_isp[DEFAULT_VALUE_SIZE];
static char buf_modem_cqi[DEFAULT_VALUE_SIZE];
static char buf_modem_ambr[DEFAULT_VALUE_SIZE];
static char buf_modem_networkmode[DEFAULT_VALUE_SIZE];
static char buf_modem_signal0name[DEFAULT_VALUE_SIZE];
static int buf_modem_signal0value = 0;
static int buf_modem_signal0min = 0;
static int buf_modem_signal0max = 0;
static char buf_modem_signal0unit[DEFAULT_VALUE_SIZE];
static char buf_modem_signal1name[DEFAULT_VALUE_SIZE];
static int buf_modem_signal1value = 0;
static int buf_modem_signal1min = 0;
static int buf_modem_signal1max = 0;
static char buf_modem_signal1unit[DEFAULT_VALUE_SIZE];
static char buf_modem_signal2name[DEFAULT_VALUE_SIZE];
static int buf_modem_signal2value = 0;
static int buf_modem_signal2min = 0;
static int buf_modem_signal2max = 0;
static char buf_modem_signal2unit[DEFAULT_VALUE_SIZE];

void parse_modem_info()
{
    strcpy(buf_modem_revision, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_temperature, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_voltage, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_connect, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_sim, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_isp, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_cqi, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_ambr, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_networkmode, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_signal0name, UNKNOWN_VALUE_REPLACE_STRING);
    buf_modem_signal0value = 0;
    buf_modem_signal0min = 0;
    buf_modem_signal0max = 0;
    strcpy(buf_modem_signal0unit, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_signal1name, UNKNOWN_VALUE_REPLACE_STRING);
    buf_modem_signal1value = 0;
    buf_modem_signal1min = 0;
    buf_modem_signal1max = 0;
    strcpy(buf_modem_signal1unit, UNKNOWN_VALUE_REPLACE_STRING);
    strcpy(buf_modem_signal2name, UNKNOWN_VALUE_REPLACE_STRING);
    buf_modem_signal2value = 0;
    buf_modem_signal2min = 0;
    buf_modem_signal2max = 0;
    strcpy(buf_modem_signal2unit, UNKNOWN_VALUE_REPLACE_STRING);
    FILE *fp;
    char line[256];
    fp = popen("/usr/bin/python3 /usr/zz/modem_info.py", "r");
    if (fp != NULL)
    {
        while (fgets(line, sizeof(line), fp) != NULL)
        {
            line[strcspn(line, "\n")] = 0;
            char *key = strtok(line, ":");
            char *value = strtok(NULL, ":");
            if (key == NULL || value == NULL)
            {
                continue;
            }
            if (strcmp(key, "revision") == 0)
            {
                strncpy(buf_modem_revision, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "temperature") == 0)
            {
                strncpy(buf_modem_temperature, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "voltage") == 0)
            {
                strncpy(buf_modem_voltage, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "connect") == 0)
            {
                strncpy(buf_modem_connect, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "sim") == 0)
            {
                strncpy(buf_modem_sim, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "isp") == 0)
            {
                strncpy(buf_modem_isp, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "cqi") == 0)
            {
                strncpy(buf_modem_cqi, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "ambr") == 0)
            {
                strncpy(buf_modem_ambr, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "networkmode") == 0)
            {
                strncpy(buf_modem_networkmode, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "signal0name") == 0)
            {
                strncpy(buf_modem_signal0name, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "signal0value") == 0)
            {
                buf_modem_signal0value = atoi(value);
            }
            else if (strcmp(key, "signal0min") == 0)
            {
                buf_modem_signal0min = atoi(value);
            }
            else if (strcmp(key, "signal0max") == 0)
            {
                buf_modem_signal0max = atoi(value);
            }
            else if (strcmp(key, "signal0unit") == 0)
            {
                strncpy(buf_modem_signal0unit, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "signal1name") == 0)
            {
                strncpy(buf_modem_signal1name, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "signal1value") == 0)
            {
                buf_modem_signal1value = atoi(value);
            }
            else if (strcmp(key, "signal1min") == 0)
            {
                buf_modem_signal1min = atoi(value);
            }
            else if (strcmp(key, "signal1max") == 0)
            {
                buf_modem_signal1max = atoi(value);
            }
            else if (strcmp(key, "signal1unit") == 0)
            {
                strncpy(buf_modem_signal1unit, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "signal2name") == 0)
            {
                strncpy(buf_modem_signal2name, value, DEFAULT_VALUE_SIZE - 1);
            }
            else if (strcmp(key, "signal2value") == 0)
            {
                buf_modem_signal2value = atoi(value);
            }
            else if (strcmp(key, "signal2min") == 0)
            {
                buf_modem_signal2min = atoi(value);
            }
            else if (strcmp(key, "signal2max") == 0)
            {
                buf_modem_signal2max = atoi(value);
            }
            else if (strcmp(key, "signal2unit") == 0)
            {
                strncpy(buf_modem_signal2unit, value, DEFAULT_VALUE_SIZE - 1);
            }
        }
    }
    pclose(fp);

    if (ui_valModemRev != NULL)
    {
        lv_label_set_text(ui_valModemRev, buf_modem_revision);
    }
    if (ui_valModemTempature != NULL)
    {
        lv_label_set_text(ui_valModemTempature, buf_modem_temperature);
    }
    if (ui_valModemVoltage != NULL)
    {
        lv_label_set_text(ui_valModemVoltage, buf_modem_voltage);
    }
    if (ui_valModemISP != NULL)
    {
        lv_label_set_text(ui_valModemISP, buf_modem_isp);
    }
    if (ui_valModemNetworkType != NULL)
    {
        lv_label_set_text(ui_valModemNetworkType, buf_modem_networkmode);
    }
    if (ui_valModemCQI != NULL)
    {
        lv_label_set_text(ui_valModemCQI, buf_modem_cqi);
    }
    if (ui_valModemAmbr != NULL)
    {
        lv_label_set_text(ui_valModemAmbr, buf_modem_ambr);
    }
    if (ui_valModemSignalName1 != NULL)
    {
        lv_label_set_text(ui_valModemSignalName1, buf_modem_signal0name);
    }
    if (ui_valModemSignalValue1 != NULL)
    {
        lv_label_set_text(ui_valModemSignalValue1, buf_modem_signal0unit);
    }
    if (ui_valModemSignalBar1 != NULL)
    {
        lv_bar_set_range(ui_valModemSignalBar1, buf_modem_signal0min, buf_modem_signal0max);
        lv_bar_set_value(ui_valModemSignalBar1, buf_modem_signal0value, LV_ANIM_OFF);
    }
    if (ui_valModemSignalName2 != NULL)
    {
        lv_label_set_text(ui_valModemSignalName2, buf_modem_signal1name);
    }
    if (ui_valModemSignalValue2 != NULL)
    {
        lv_label_set_text(ui_valModemSignalValue2, buf_modem_signal1unit);
    }
    if (ui_valModemSignalBar2 != NULL)
    {
        lv_bar_set_range(ui_valModemSignalBar2, buf_modem_signal1min, buf_modem_signal1max);
        lv_bar_set_value(ui_valModemSignalBar2, buf_modem_signal1value, LV_ANIM_OFF);
    }
    if (ui_valModemSignalName3 != NULL)
    {
        lv_label_set_text(ui_valModemSignalName3, buf_modem_signal2name);
    }
    if (ui_valModemSignalValue3 != NULL)
    {
        lv_label_set_text(ui_valModemSignalValue3, buf_modem_signal2unit);
    }
    if (ui_valModemSignalBar3 != NULL)
    {
        lv_bar_set_range(ui_valModemSignalBar3, buf_modem_signal2min, buf_modem_signal2max);
        lv_bar_set_value(ui_valModemSignalBar3, buf_modem_signal2value, LV_ANIM_OFF);
    }
}

static void update_static_value(void)
{
    struct utsname info;
    if (uname(&info) == -1)
    {
        strcpy(buf_kernel_version, UNKNOWN_VALUE_REPLACE_STRING);
    }
    else
    {
        strcpy(buf_kernel_version, info.release);
    }
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    memory_total_bytes = pages * page_size;
    format_memory_size(memory_total_bytes, buf_memory_total_bytes);
}

static void update_screen_data(void)
{
    if (ui_valHostname != NULL)
    {
        if (gethostname(buf_hostname, DEFAULT_VALUE_SIZE))
        {
            strcpy(buf_hostname, UNKNOWN_VALUE_REPLACE_STRING);
        }
        lv_label_set_text(ui_valHostname, buf_hostname);
    }
    if (ui_valSysVersion != NULL && ui_valBuildId != NULL)
    {
        read_os_release(buf_sys_version, DEFAULT_VALUE_SIZE, buf_build_id, DEFAULT_VALUE_SIZE);
        lv_label_set_text(ui_valSysVersion, buf_sys_version);
        lv_label_set_text(ui_valBuildId, buf_build_id);
    }
    if (ui_valKernelVersion != NULL)
    {
        lv_label_set_text(ui_valKernelVersion, buf_kernel_version);
    }
    if (ui_valLoadAvg != NULL)
    {
        char tmp_load_avg[DEFAULT_VALUE_SIZE];
        float avg_1, avg_5, avg_15;
        read_file_to_string(tmp_load_avg, sizeof(tmp_load_avg), "/proc/loadavg");
        if (sscanf(tmp_load_avg, "%f %f %f", &avg_1, &avg_5, &avg_15) != 3)
        {
            strcpy(buf_load_avg, UNKNOWN_VALUE_REPLACE_STRING);
        }
        else
        {
            snprintf(buf_load_avg, sizeof(buf_load_avg), "%.2f / %.2f / %.2f", avg_1, avg_5, avg_15);
        }
        lv_label_set_text(ui_valLoadAvg, buf_load_avg);
    }
    if (ui_valMemory != NULL)
    {
        FILE *fp = fopen("/proc/meminfo", "r");
        if (fp)
        {
            unsigned long memory_free_bytes = 0;
            char line[256];
            while (fgets(line, sizeof(line), fp))
            {
                if (strstr(line, "MemFree:"))
                {
                    sscanf(line, "MemFree: %lu kB", &memory_free_bytes);
                    memory_free_bytes *= 1024;
                    break;
                }
            }
            unsigned long memory_used_bytes = memory_total_bytes - memory_free_bytes;
            double usage_percent = (double)memory_used_bytes / memory_total_bytes * 100;
            char buf_used_str[32];
            format_memory_size(memory_used_bytes, buf_used_str);
            snprintf(buf_memory, sizeof(buf_memory), "%s / %s (%.0f%%)",
                     buf_used_str, buf_memory_total_bytes, usage_percent);
            lv_label_set_text(ui_valMemory, buf_memory);
        }
        else
        {
            lv_label_set_text(ui_valMemory, UNKNOWN_VALUE_REPLACE_STRING);
        }

        fclose(fp);
    }
    if (ui_valUptime != NULL)
    {
        struct sysinfo info;
        if (sysinfo(&info) == 0)
        {
            long uptime = info.uptime;
            int days = uptime / (24 * 3600);
            uptime %= (24 * 3600);
            int hours = uptime / 3600;
            uptime %= 3600;
            int minutes = uptime / 60;
            int seconds = uptime % 60;
            snprintf(buf_uptime, DEFAULT_VALUE_SIZE, "%d 天 %d 小时 %d 分 %d 秒",
                     days, hours, minutes, seconds);
            lv_label_set_text(ui_valUptime, buf_uptime);
        }
        else
        {
            lv_label_set_text(ui_valUptime, UNKNOWN_VALUE_REPLACE_STRING);
        }
    }
    if (ui_valLocalTime != NULL)
    {
        time_t raw_time;
        struct tm *time_info;
        time(&raw_time);
        time_info = localtime(&raw_time);
        strftime(buf_local_time, sizeof(buf_local_time), "%Y-%m-%d %H:%M:%S", time_info);
        lv_label_set_text(ui_valLocalTime, buf_local_time);
    }
    if (ui_valModemIp != NULL)
    {
        if (get_first_wwan_ipv4_address(buf_modem_ip, sizeof(buf_modem_ip)) == 0)
        {
            lv_label_set_text(ui_valModemIp, buf_modem_ip);
        }
        else
        {
            lv_label_set_text(ui_valModemIp, UNKNOWN_IP_REPLACE_STRING);
        }
    }
    if (ui_valWanIp != NULL)
    {
        if (get_interface_ipv4_address("eth1", buf_wan_ip, sizeof(buf_wan_ip)) == 0)
        {
            lv_label_set_text(ui_valWanIp, buf_wan_ip);
        }
        else
        {
            lv_label_set_text(ui_valWanIp, UNKNOWN_IP_REPLACE_STRING);
        }
    }
    if (ui_valLanIp != NULL)
    {
        if (get_interface_ipv4_address("br-lan", buf_lan_ip, sizeof(buf_lan_ip)) == 0)
        {
            lv_label_set_text(ui_valLanIp, buf_lan_ip);
        }
        else
        {
            lv_label_set_text(ui_valLanIp, UNKNOWN_IP_REPLACE_STRING);
        }
    }
    if (ui_valActiveConnect != NULL)
    {
        snprintf(buf_active_connect, DEFAULT_VALUE_SIZE, "%d", get_nf_conntrack_count());
        lv_label_set_text(ui_valActiveConnect, buf_active_connect);
    }
    if (ui_valArpCount != NULL)
    {
        snprintf(buf_arp_count, DEFAULT_VALUE_SIZE, "%d", count_arp_online());
        lv_label_set_text(ui_valArpCount, buf_arp_count);
    }
}

int main(void)
{
    lv_init();

    /*Linux display device init*/
    lv_linux_disp_init();

    ui_init();
    /*Handle LVGL tasks*/
    update_static_value();
    while (1)
    {
        lv_timer_handler();
        usleep(5000);
        update_screen_data_time_counter++;
        if (update_screen_data_time_counter >= 200)
        {
            update_screen_data_time_counter = 0;
            update_modem_data_time_counter += 1;
            update_screen_data();
        }
        if (update_modem_data_time_counter >= 30)
        {
            update_modem_data_time_counter = 0;
            parse_modem_info();
        }
    }

    return 0;
}